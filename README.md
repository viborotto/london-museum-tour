# london-museum-tour 

## Atividade 2 | Computação Gráfica:  desenvolvimento de uma aplicação gráfica 3D



### Autores:    

🧑  *Felipe Moreira Temoteo da Silva*   RA: 11201811314 <BR>
👩  *Vittoria Ariel dos Santos Borotto* RA: 11201811288   <BR> 

### Por que a escolha de um Tour pelo Museu de Londres ? 

Devido a um dos integrantes da dupla ter feito uma matéria na UFABC chamada "Educação Científica, Sociedade e Cultura", a qual envolvia uma visita a um espaço informal, que no caso foi um Museu virtual por conta da pandemia, surgiu a ideia de um Tour Virtual em um Museu. 

Achamos que seria interessante seguir essa ideia, puxando para o lado de interdisciplinaridade, e como estamos tambem estudando sobre Didática no atual quadrimestre, **juntaremos cultura, educação, ciências, e computação gráfica** nessa atividade 2.

### O que é um Tour pelo Museu de Historia Natural de Londres ? 

Normalmente a visita virtual em um Museu, consiste em andar sobre o museu, visualizando exposiçōes, e ao chegar perto de alguma obra, ou objeto conseguir aprender sobre aquilo que está exposto.

### Museu de Historia Natural de Londres (NHM)

Ande sob o maior animal da Terra e explore dezenas de outras exposições que representam 4,5 bilhões de anos de história natural.

O Museu apresentado é a porta de entrada para novos conhecimentos, nele, você pode passear entre meteoritos, mamíferos, peixes, pássaros, minerais, plantas e insetos.

**Conceitos utilizados durante a atividade 2** 💻:
- Representação vetorial no OpenGL (GLTRIANGLES) <BR>
	◼️ A representação vetorial é usada para definir a geometria que será usada processada durante toda a renderização, e pode ser vista na formação das primitivas que compõem o set MandelBrot. <BR>
	
- Dispositivos de E/S(Teclado,mouse e monitor): <BR>
	◼️ Durante a atividade foram utilizados os conceitos de Dispositivos de entrada e saída. <BR>
	◼️ Dispositivos de entrada: temos como exemplo as setas e as letras F,V,B,C que ao utilizar um deles você interage com o programa navegando pelo espaço 2D do set.<BR>
	◼️ Dispositivos de saída: Toda interação com a atividade, reflete em mudanças que são exibidas no monitor do usuário.<BR>
	◼️ Processadores: Para os diversos processamentos do set se faz necessário o uso de CPU's,GPU's e seus subsistemas.<BR>
- Framebuffer <BR>
	◼️ Frame Buffer é uma memória especializada em armazenar e transferir para a tela do computador dados de um quadro de imagem. As informações armazenadas nesta memória consistem basicamente em valores cromáticos para cada pixel e suas transparências. Quanto maior a resolução da imagem retratada maior será a memória necessária para o frame Buffer armazenar as imagens.<BR>
- Rasterização <BR>
	◼️ Compreende a conversão matricial das primitivas. O resultado é um conjunto de amostras de primitivas. Durante o processamento no pipeline, o termo fragmento é frequentemente utilizado para designar essas amostras no lugar de pixel. Cada fragmento é uma coleção de valores que inclui atributos interpolados a partir dos vértices e a posição (x,y,z)da amostra em coordenadas da janela (o valor z é considerado a “profundidade” do fragmento). <BR>
- Vertex shader <BR>
	◼️ O vertex shader é uma função gráfica responsável por adicionar efeitos especiais a objetos em ambientes 3D, conforme executa operações matemáticas sobre cada vértice formador da imagem. Cada vértice é então definido em diversas variáveis, sendo basicamente descrito pelo seu posicionamento no espaço tridimensional. O vertex shader processa cada vértice individualmente. Entretanto, esse processamento é paralelizado de forma massiva na GPU. Cada execução de um vertex shader acessa apenas os atributos do vértice que está sendo processado. Não há como compartilhar o estado do processamento de um vértice com os demais vértices.<BR>
- Fragment shader <BR>
	◼️ O fragment shader é um programa que processa cada fragmento individualmente após a rasterização. A entrada do fragment shader é o mesmo conjunto de atributos definidos pelo usuário na saída do vertex shader.
- VBO <BR>
	◼️ O Vertex buffer object (VBO) permite que arrays de vértices sejam armazenados na memória gráfica de alta performance do lado do servidor e promove transferência eficiência de dados.<BR>

__A aplicação desenvolvida consiste em uma navegação pelo Museu de História natural de Londres.__

#### Interação durante o Tour no Museu: 
			
1. Abra a aplicação localmente 
2. Para interagir aperte as seguintes teclas: 
    -  Seta para cima ou W: ande para frente
    -  Seta para baixo ou S: ande para trás
    -  Seta para a direta ou D: ande para a direita
    -  Seta para a esquerda ou A: ande para a esquerda
	
#### Para mais informaçōes sobre o Museu de Historia Natural de Londres:  
https://en.wikipedia.org/wiki/Natural_History_Museum,_London

#### Referência modelo 3d:
https://sketchfab.com/3d-models/hintze-hall-nhm-london-surface-model-b2f3e84112d04bf1844e7ac2c4423566