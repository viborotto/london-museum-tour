#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imfilebrowser.h"

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w)
      m_dollySpeed = 0.5f;
    if (ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s)
      m_dollySpeed = -0.5f;
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
      m_panSpeed = -0.5f;
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
      m_panSpeed = 0.5f;
    if (ev.key.keysym.sym == SDLK_q) m_truckSpeed = -0.5f;
    if (ev.key.keysym.sym == SDLK_e) m_truckSpeed = 0.5f;
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
      ImGui::Begin("Instru????es", &firstExec);

      ImGui::Text("Bem-vindo ao Museu de Hist??ria Natural de Londres");
      ImGui::Text("Para explorar e aprender bastar andar pelo museu, e ler sobre suas exposi??oes");
      ImGui::Text("Comandos:");
      ImGui::Text("Girar a c??mera: A e D");
      ImGui::Text("Andar: W, S, Q e E");
      ImGui::Text("Ap??s visualizar uma exposi??ao, clique com o mouse para liberar a c??mera");
      // ImGui::Text("Shift: Agachar");
      // ImGui::Text("Espa??o: Interagir");

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
    if (m_camera.m_eye[0] < 0.02 && m_camera.m_eye[0] > -0.02 && m_camera.m_eye[1] < 0.29 && m_camera.m_eye[1] > 0.16) {
        
        if(exp5) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposi????o 5 - ??rvores F??sseis", &exp5);
        ImGui::Text("Juntos, eles abrangem centenas de milh??es de anos de hist??ria da Terra, desde a ??rvore de 358 milh??es ");
        ImGui::Text("de anos na frente da caixa at?? o esp??cime comparativamente mais jovem"); 
        ImGui::Text("na parte traseira, que tem entre 23 e 65 milh??es de anos.");
        ImGui::Text("A girafa ?? o mais alto de todos os animais vivos. Ele pode atingir quase seis metros acima do solo");
        ImGui::Text("e pode faz??-lo porque suas pernas e pesco??o s??o muito alongados em compara????o com o resto do corpo.");
        m_truckSpeed=0.0f;
        m_dollySpeed=0.0f;
        m_panSpeed=0.0f;
        ImGui::End();
      }
    }
    if (m_camera.m_eye[0] < -0.32 && m_camera.m_eye[0] > -0.38 && m_camera.m_eye[1] < -0.16 && m_camera.m_eye[1] > -0.35) {
        
        if(exp10) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposi????o 10 -  Algas marinhas", &exp10);

        ImGui::Text("Eles podem n??o parecer muito quando levados para a praia, mas as algas marinhas fornecem um ");
        ImGui::Text("habitat subaqu??tico vital.");
        ImGui::Text("A professora Juliet Brodie est?? lan??ando luz sobre as vastas florestas que crescem em nossos oceanos.");
        ImGui::Text("Existem muitas esp??cies de algas marinhas que chamam de lar os mares frios da Gr??-Bretanha."); 
        ImGui::Text("O maior deles s??o kelps.Essas algas marrons crescem da costa at?? 20-30 metros, ou mais se a ??gua"); 
        ImGui::Text(" estiver limpa. Eles formam florestas densas e fornecem um habitat para uma diversidade de vida marinha.");
        ImGui::Text("Juliet diz: ???Eles podem ser viveiros de peixes e fornecer servi??os para muitos outros tipos diferentes");
        ImGui::Text("de animais e algas marinhas. A floresta est?? cheia de toda essa vida incr??vel.");
        ImGui::Text("' At?? mesmo o holdfast - a estrutura que conecta grandes algas marrons aos fundos");
        ImGui::Text("marinhos rochosos - sustenta uma grande quantidade de vida.");
        m_truckSpeed=0.0f;
        m_dollySpeed=0.0f;
        m_panSpeed=0.0f;
        ImGui::End();
      }
      }
      if (m_camera.m_eye[0] < -0.14 && m_camera.m_eye[0] > -0.30 && m_camera.m_eye[1] < 0.30  && m_camera.m_eye[1] > 0.14) {
        
        if(exp6) {
        auto widgetSize{ImVec2(800, 250)};
        ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                      (m_viewportHeight - widgetSize.y) / 2));
        ImGui::SetNextWindowSize(widgetSize);
        ImGui::Begin("Exposi????o 6 - Fomara????o de ferro em faixas", &exp6);

        ImGui::Text("Forma????o de ferro em faixas Mais de tr??s bilh??es de anos atr??s, as bact??rias no oceano jovem da Terra ");
        ImGui::Text("come??aram a produzir oxig??nio por meio da fotoss??ntese.");
        ImGui::Text("Cerca de 2,6 bilh??es de anos atr??s, as bact??rias realmente come??aram a florescer e os n??veis ");
        ImGui::Text("de oxig??nio seguiram o exemplo."); 
        ImGui::Text("Esse oxig??nio se combinou com o ferro dissolvido no mar para formar ??xido de ferro insol??vel,");
        ImGui::Text("que afundou no fundo do mar. ?? medida que assentava, folhas de ??xido de ferro vermelho ");
        ImGui::Text("foram colocadas entre camadas de lodo rico em s??lica.");
        ImGui::Text("Ao longo de centenas de milh??es de anos, o oxig??nio se ligou a todo o ferro sol??vel nas ??guas.");
        ImGui::Text("O oxig??nio livre restante n??o tinha outro lugar para ir, a n??o ser para cima e para fora na atmosfera.");
        ImGui::Text("As camadas intrincadas na forma????o representam um ponto de viragem na hist??ria da Terra conhecido ");
        ImGui::Text("como o Grande Evento de Oxigena????o.");
        m_truckSpeed=0.0f;
        m_dollySpeed=0.0f;
        m_panSpeed=0.0f;
        ImGui::End();
      }
    }
  if (m_camera.m_eye[0] < 0.35 && m_camera.m_eye[0] > 0.28 && m_camera.m_eye[1] < 0.35 && m_camera.m_eye[1] > 0.13) {
          
          if(exp3) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 3 - Missouri Leviathan", &exp3);

          ImGui::Text("O mastodonte americano foi um grande mam??fero terrestre que vagou pela Am??rica do Norte durante ");
          ImGui::Text("a Idade do Gelo at?? 13.000 anos atr??s.");
          ImGui::Text("Mastodontes viviam em florestas de pinheiros e ??reas pantanosas cobertas por lar??cio e abetos, ");
          ImGui::Text("alimentando-se de galhos, folhas e plantas aqu??ticas.");
          ImGui::Text("Adaptados para a vida na beira da ??gua, eles tinham p??s largos e dedos dos p??s atarracados");
          ImGui::Text(" e bem abertos. Isso permitiu que eles andassem no solo macio e alagado ao lado de lagoas e lagos.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
          ImGui::End();
        }
      }
       if (m_camera.m_eye[0] < -0.47 && m_camera.m_eye[0] > -0.51 && m_camera.m_eye[1] < 0.30 && m_camera.m_eye[1] > 0.14) {
          
          if(exp7) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 7 - Meteorito Imilac", &exp7);

          ImGui::Text("Esta pe??a extraterrestre faz parte de um antigo meteorito palasita. ?? uma fatia de um dos ");
          ImGui::Text("maiores esp??cimes de seu tipo no mundo.");
          ImGui::Text("Acredita-se que ele tenha feito parte de um meteoro muito maior que pesava at?? 1.000 kg ");
          ImGui::Text("e explodiu sobre o Deserto de Atacama, ");
          ImGui::Text("no norte do Chile, possivelmente no s??culo XIV.Ap??s a explos??o, ");
          ImGui::Text("fragmentos de v??rios tamanhos foram espalhados por uma ");
          ImGui::Text("vasta ??rea de deserto ??rido.");
          ImGui::Text("O meteorito n??o ?? apenas bonito, ele cont??m informa????es sobre a hist??ria inicial ");
          ImGui::Text("de nosso pr??prio planeta, desde o in??cio do sistema solar.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
          ImGui::End();
          
        }
          
      }
      if (m_camera.m_eye[0] < 0.18 && m_camera.m_eye[0] > 0.13 && m_camera.m_eye[1] < 0.34 && m_camera.m_eye[1] > 0.14) {
          
          if(exp4) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 4 - Esqueleto de Mantellisaurus", &exp4);
          ImGui::Text("O iguanodonte foi um dos tr??s primeiros dinossauros a ser descoberto, mas o famoso r??ptil pode ter");
          ImGui::Text("arrastado v??rios esqueletos identificados erroneamente em seu rastro.");
          ImGui::Text("Um dos esqueletos mais completos do mundo de Mantellisaurus atherfieldensis est?? em exibi????o no Hintze");
          ImGui::Text("Hall do Museu, recentemente remodelado.");
          ImGui::Text("No entanto, o dinossauro s?? recentemente reivindicou sua verdadeira identidade,");
          ImGui::Text("ap??s passar mais de 80 anos conhecido pelo mundo como uma esp??cie de iguanodonte.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
          ImGui::End();
        }
      }
        if (m_camera.m_eye[0] < -0.49 && m_camera.m_eye[0] > -0.57 && m_camera.m_eye[1] < -0.19 && m_camera.m_eye[1] > -0.35  ) {
          
          if(exp9) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 9 - insetos", &exp9);
          ImGui::Text("O n??mero de insetos no mundo ?? enorme. Existem cerca de cinco e meio milh??es de esp??cies diferentes");
          ImGui::Text(" de insetos e muitos milh??es de indiv??duos de qualquer uma dessas esp??cies.");
          ImGui::Text("Portanto, n??o ?? de surpreender que os insetos como grupo tenham um efeito amplo e profundo ");
          ImGui::Text("no mundo ao nosso redor.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
          ImGui::End();
        }
      }

       if (m_camera.m_eye[0] < 0.01 && m_camera.m_eye[0] > -0.06 && m_camera.m_eye[1] < -0.18 && m_camera.m_eye[1] > -0.35) {
          
          if(exp11) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 11 - Turbinaria bifrons", &exp11);
          ImGui::Text("Um coral antigo branqueado ?? uma adi????o instigante ao Hintze Hall.Pesando mais de 300 quilos, o gigante ");
          ImGui::Text("Turbinaria bifrons foi coletado no Shark Bay Reef, na costa da Austr??lia Ocidental, h?? mais de 120 anos.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
          ImGui::End();
        }
      }

      if (m_camera.m_eye[0] < 0.21 && m_camera.m_eye[0] > 0.14 && m_camera.m_eye[1] < -0.19 && m_camera.m_eye[1] > -0.35) {
          
          if(exp12) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 12 - Marlin azul do atl??ntico", &exp12);
          ImGui::Text("O peixe de quatro metros de comprimento foi descoberto em uma praia de Pembrokeshire na semana passada.");
          ImGui::Text("Embora algumas pessoas tenham pensado inicialmente que era um peixe-espada, ele foi identificado ");
          ImGui::Text("como um marlin azul - apenas o terceiro foi encontrado no Reino Unido.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
          ImGui::End();
        }
      }

       if (m_camera.m_eye[0] < 0.35 && m_camera.m_eye[0] > 0.29 && m_camera.m_eye[1] < -0.18 && m_camera.m_eye[1] > -0.35) {
          
          if(exp13) {
          auto widgetSize{ImVec2(800, 250)};
          ImGui::SetNextWindowPos(ImVec2((m_viewportWidth - widgetSize.x) / 2,
                                        (m_viewportHeight - widgetSize.y) / 2));
          ImGui::SetNextWindowSize(widgetSize);
          ImGui::Begin("Exposi????o 13 - Girafa", &exp13);
          ImGui::Text("A girafa ?? o mais alto de todos os animais vivos. Ele pode atingir quase seis metros acima do solo ");
          ImGui::Text("e pode faz??-lo porque suas pernas e pesco??o s??o muito alongados em compara????o com o resto do corpo.");
          m_truckSpeed=0.0f;
          m_dollySpeed=0.0f;
          m_panSpeed=0.0f;
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