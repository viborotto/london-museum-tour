#include "openglwindow.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}  // namespace std

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_q) m_truckSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_e) m_truckSpeed = 1.0f;
  }
  if (ev.type == SDL_KEYUP) {
    if ((ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_q && m_truckSpeed < 0) m_truckSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_e && m_truckSpeed > 0) m_truckSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "lookat.vert",
                                    getAssetsPath() + "lookat.frag");


  // Load model
  loadModelFromFile(getAssetsPath() + "Museu.obj");

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  const GLint positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::loadModelFromFile(std::string_view path) {
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (const auto& shape : shapes) {
    // Loop over indices
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      const tinyobj::index_t index{shape.mesh.indices.at(offset)};

      // Vertex position
      const int startIndex{3 * index.vertex_index};
      const float vx{attrib.vertices.at(startIndex + 0)};
      const float vy{attrib.vertices.at(startIndex + 1)};
      const float vz{attrib.vertices.at(startIndex + 2)};

      Vertex vertex{};
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables (could be precomputed)
  const GLint viewMatrixLoc{
      abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{
      abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE,
                           &m_camera.m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE,
                           &m_camera.m_projMatrix[0][0]);

  abcg::glBindVertexArray(m_VAO);

  // Draw white bunny
  glm::mat4 model{1.0f};
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(0.5f));

  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
  abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                       nullptr);

  abcg::glBindVertexArray(0);


  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() { 
  abcg::OpenGLWindow::paintUI(); 
  {
    if(firstExec) {
      auto widgetSize{ImVec2(800, 250)};
      ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                    (m_viewportHeight - widgetSize.y) / 2));
      ImGui::SetNextWindowSize(widgetSize);
      ImGui::Begin("Instruções", &firstExec);

      ImGui::Text("Bem-vindo ao Museu de História Natural de Londres");
      ImGui::Text("Para explorar e aprender bastar andar pelo museu, e ler sobre suas exposiçōes");
      ImGui::Text("Comandos:");
      ImGui::Text("Girar a câmera: A e D");
      ImGui::Text("Andar: W, S, Q e E");
      // ImGui::Text("Shift: Agachar");
      // ImGui::Text("Espaço: Interagir");

      ImGui::End();
    }
    {
      auto widgetSizeB{ImVec2(222, 100)};
    // Slider to control light properties
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSizeB.x - 50,
                                   m_viewportHeight - widgetSizeB.y - 50));
    ImGui::SetNextWindowSize(widgetSizeB);
    ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::Text("%f", m_camera.m_eye[0]);
    ImGui::Text("%f", m_camera.m_eye[1]);
    ImGui::Text("%f", m_camera.m_eye[2]);
    if (m_camera.m_eye[1] < 2.90 && m_camera.m_eye[1] > 2.31 && m_camera.m_eye[2] < 1.10 && m_camera.m_eye[2] > -0.51) {
      ImGui::Text("First Exposition");
    }
    ImGui::End();
    }
    {
      // Marlin Azul do Atlantico
    if (m_camera.m_eye[1] < 2.90 && m_camera.m_eye[1] > 2.31 && m_camera.m_eye[2] < 1.10 && m_camera.m_eye[2] > -0.51) {
        
        if(exp1) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposição 1", &exp1);

        ImGui::Text("Marlin azul do atlantico, O peixe de quatro metros de comprimento foi descoberto em uma praia de Pembrokeshire.");
        ImGui::Text("Embora algumas pessoas tenham pensado inicialmente que era um peixe-espada, ele foi identificado como um marlin azul - ");
        ImGui::Text("apenas o terceiro que foi encontrado no Reino Unido.");
        ImGui::Text("Após a inspeção na praia, o Conselho do Condado de Pembrokeshire transferiu o peixe para um armazenamento temporário");
        ImGui::Text("e relatou a descoberta ao Programa de Investigação de Cetáceos do Reino Unido, que notificou o Museu.");
        ImGui::End();
      }
    }
  }
  {
// Girafa - em ajuste
 if (m_camera.m_eye[1] < -2.55 && m_camera.m_eye[1] > -2.70 && m_camera.m_eye[2] < 1.10 && m_camera.m_eye[2] > -3.68) {        
        if(exp2) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposição 2", &exp2);

        ImGui::Text("A girafa é o mais alto de todos os animais vivos. Ele pode atingir quase seis metros acima do solo");
        ImGui::Text("e pode fazê-lo porque suas pernas e pescoço são muito alongados em comparação com o resto do corpo.");
        ImGui::End();
      }
    }
  }
  }
  

}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {

  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
}