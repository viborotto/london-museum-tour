#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imfilebrowser.h"

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
  abcg::glEnable(GL_DEPTH_TEST);

  // Create programs
  m_program = createProgramFromFile(getAssetsPath() + "shaders/texture.vert",
                                    getAssetsPath() + "shaders/texture.frag");

  // Load default model
  loadModel(getAssetsPath() + "hintze-hall-1m.obj");
  m_mappingMode = 3;

}

void OpenGLWindow::loadModel(std::string_view path) {
  m_model.terminateGL();

  m_model.loadDiffuseTexture(getAssetsPath() + "hintze-hall-1m_u1_v1.jpg");
  m_model.loadObj(path);
  m_model.setupVAO(m_program);
  m_trianglesToDraw = m_model.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model.getKa();
  m_Kd = m_model.getKd();
  m_Ks = m_model.getKs();
  m_shininess = m_model.getShininess();
}


void OpenGLWindow::paintGL() {
  update();

  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // Use currently selected program
  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  const GLint viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint shininessLoc{abcg::glGetUniformLocation(m_program, "shininess")};
  const GLint IaLoc{abcg::glGetUniformLocation(m_program, "Ia")};
  const GLint IdLoc{abcg::glGetUniformLocation(m_program, "Id")};
  const GLint IsLoc{abcg::glGetUniformLocation(m_program, "Is")};
  const GLint KaLoc{abcg::glGetUniformLocation(m_program, "Ka")};
  const GLint KdLoc{abcg::glGetUniformLocation(m_program, "Kd")};
  const GLint KsLoc{abcg::glGetUniformLocation(m_program, "Ks")};
  const GLint diffuseTexLoc{abcg::glGetUniformLocation(m_program, "diffuseTex")};
  const GLint mappingModeLoc{
      abcg::glGetUniformLocation(m_program, "mappingMode")};

  // Set uniform variables used by every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);

  // abcg::glUniformMatrix3fv(texMatrixLoc, 1, GL_TRUE, &texMatrix[0][0]);

  abcg::glUniform4fv(IaLoc, 1, &m_Ia.x);
  abcg::glUniform4fv(IdLoc, 1, &m_Id.x);
  abcg::glUniform4fv(IsLoc, 1, &m_Is.x);

  // Set uniform variables of the current object
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  const auto modelViewMatrix{glm::mat3(m_viewMatrix * m_modelMatrix)};

  abcg::glUniform1f(shininessLoc, m_shininess);
  abcg::glUniform4fv(KaLoc, 1, &m_Ka.x);
  abcg::glUniform4fv(KdLoc, 1, &m_Kd.x);
  abcg::glUniform4fv(KsLoc, 1, &m_Ks.x);

  m_model.render(m_trianglesToDraw);

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
        ImGui::Begin("Exposição 5 - Árvores Fósseis", &exp5);

        ImGui::Text("Juntos, eles abrangem centenas de milhões de anos de história da Terra, desde a árvore de 358 milhões ");
        ImGui::Text("de anos na frente da caixa até o espécime comparativamente mais jovem"); 
        ImGui::Text("na parte traseira, que tem entre 23 e 65 milhões de anos.");

        ImGui::Text("A girafa é o mais alto de todos os animais vivos. Ele pode atingir quase seis metros acima do solo");
        ImGui::Text("e pode fazê-lo porque suas pernas e pescoço são muito alongados em comparação com o resto do corpo.");
        ImGui::End();
      }
    }
    if (m_camera.m_eye[1] < -2.5 && m_camera.m_eye[1] > -3 && m_camera.m_eye[2] < 5 && m_camera.m_eye[2] > 4.5) {
        
        if(exp10) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposição 10 -  Algas marinhas", &exp10);

        ImGui::Text("Eles podem não parecer muito quando levados para a praia, mas as algas marinhas fornecem um habitat subaquático vital. ");
        ImGui::Text("A professora Juliet Brodie está lançando luz sobre as vastas florestas que crescem em nossos oceanos.");
        ImGui::Text("Existem muitas espécies de algas marinhas que chamam de lar os mares frios da Grã-Bretanha."); 
        ImGui::Text("O maior deles são kelps.Essas algas marrons crescem da costa até 20-30 metros, ou mais se a água estiver limpa. "); 
        ImGui::Text("Eles formam florestas densas e fornecem um habitat para uma diversidade de vida marinha.");
        ImGui::Text("Juliet diz: ‘Eles podem ser viveiros de peixes e fornecer serviços para muitos outros tipos diferentes");
        ImGui::Text("de animais e algas marinhas. A floresta está cheia de toda essa vida incrível. ");
        ImGui::Text("' Até mesmo o holdfast - a estrutura que conecta grandes algas marrons aos fundos ");
        ImGui::Text("marinhos rochosos - sustenta uma grande quantidade de vida.");
        ImGui::End();
      }
      }
      if (m_camera.m_eye[1] < 3 && m_camera.m_eye[1] > 2 && m_camera.m_eye[2] < 5 && m_camera.m_eye[2] > 4.5) {
        
        if(exp6) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposição 6 - Fomaração de ferro em faixas", &exp6);

        ImGui::Text("Formação de ferro em faixas Mais de três bilhões de anos atrás, as bactérias no oceano jovem da Terra ");
        ImGui::Text("começaram a produzir oxigênio por meio da fotossíntese.");
        ImGui::Text("Cerca de 2,6 bilhões de anos atrás, as bactérias realmente começaram a florescer e os níveis ");
        ImGui::Text("de oxigênio seguiram o exemplo."); 
        ImGui::Text("Esse oxigênio se combinou com o ferro dissolvido no mar para formar óxido de ferro insolúvel,");
        ImGui::Text("que afundou no fundo do mar. À medida que assentava, folhas de óxido de ferro vermelho ");
        ImGui::Text("foram colocadas entre camadas de lodo rico em sílica.");
        ImGui::Text("Ao longo de centenas de milhões de anos, o oxigênio se ligou a todo o ferro solúvel disponível nas águas.");
        ImGui::Text("O oxigênio livre restante não tinha outro lugar para ir, a não ser para cima e para fora na atmosfera.");
        ImGui::Text("As camadas intrincadas na formação representam um ponto de viragem na história da Terra conhecido ");
        ImGui::Text("como o Grande Evento de Oxigenação.");

        ImGui::End();
      }
    }
  if (m_camera.m_eye[1] < 2.1 && m_camera.m_eye[1] > 2 && m_camera.m_eye[2] < -3 && m_camera.m_eye[2] > -4.5) {
          
          if(exp3) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposição 3 - Missouri Leviathan", &exp3);

          ImGui::Text("O mastodonte americano foi um grande mamífero terrestre que vagou pela América do Norte durante ");
          ImGui::Text("a Idade do Gelo até 13.000 anos atrás.");
          ImGui::Text("Mastodontes viviam em florestas de pinheiros e áreas pantanosas cobertas por larício e abetos, ");
          ImGui::Text("alimentando-se de galhos, folhas e plantas aquáticas.");
          ImGui::Text("Adaptados para a vida na beira da água, eles tinham pés largos e dedos dos pés atarracados e bem abertos.");
          ImGui::Text("Isso permitiu que eles andassem no solo macio e alagado ao lado de lagoas e lagos.");

          ImGui::End();
        }
      }
       if (m_camera.m_eye[1] < 3 && m_camera.m_eye[1] > 2.1 && m_camera.m_eye[2] < 7.6 && m_camera.m_eye[2] > 6.5) {
          
          if(exp7) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposição 7 - Meteorito Imilac", &exp7);

          ImGui::Text("Esta peça extraterrestre faz parte de um antigo meteorito palasita. É uma fatia de um dos ");
          ImGui::Text("maiores espécimes de seu tipo no mundo.");
          ImGui::Text("Acredita-se que ele tenha feito parte de um meteoro muito maior que pesava até 1.000 kg ");
          ImGui::Text("e explodiu sobre o Deserto de Atacama, ");
          ImGui::Text("no norte do Chile, possivelmente no século XIV.Após a explosão, ");
          ImGui::Text("fragmentos de vários tamanhos foram espalhados por uma ");
          ImGui::Text("vasta área de deserto árido.");
          ImGui::Text("O meteorito não é apenas bonito, ele contém informações sobre a história inicial ");
          ImGui::Text("de nosso próprio planeta, desde o início do sistema solar.");
          

          ImGui::End();
        }
      }
      if (m_camera.m_eye[1] < 2 && m_camera.m_eye[1] > 2.1 && m_camera.m_eye[2] < -1 && m_camera.m_eye[2] > -2.5) {
          
          if(exp4) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposição 4 - Esqueleto de Mantellisaurus", &exp4);
          ImGui::Text("O iguanodonte foi um dos três primeiros dinossauros a ser descoberto, mas o famoso réptil pode ter arrastado vários esqueletos ");
          ImGui::Text("identificados erroneamente em seu rastro.");
          ImGui::Text("Um dos esqueletos mais completos do mundo de Mantellisaurus atherfieldensis está em exibição no Hintze Hall do Museu, ");
          ImGui::Text("recentemente remodelado.");
          ImGui::Text("No entanto, o dinossauro só recentemente reivindicou sua verdadeira identidade, após passar mais de 80 anos conhecido ");
          ImGui::Text("pelo mundo como uma espécie de iguanodonte.");

          

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
  m_model.terminateGL();
    abcg::glDeleteProgram(m_program);
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
}