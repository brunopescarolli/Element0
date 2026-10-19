# Design Document — Element 0

## 1. Objetivo Educativo
Ensinar na prática as propriedades físicas e químicas dos elementos da tabela periódica (pontos de fusão e ebulição, estados físicos a 20 °C, dureza, magnetismo e radioatividade) e demonstrar como a transferência de calor altera o estado da matéria (sólido, líquido e gasoso).

## 2. Controles
- **A / D**: Mover para esquerda / direita
- **S**: Descer de plataformas atravessáveis e rampas
- **Espaço**: Pular
- **W**: Interagir / Entrar em portas
- **E / Q**: Pegar / Soltar equipamento (Mop)
- **F ou Botão Esquerdo do Mouse**: Disparar projétil com a ferramenta equipada
- **F3 + B**: Alternar Modo Debug (visualização de hitboxes e temperaturas em tempo real)
- **ESC**: Pausar jogo / Navegar nos menus

## 3. Sistemas e Reações Químicas
- **Ambiente Base**: A estação espacial mantém temperatura ambiente constante de **20.0 °C**.
- **Corpo do Jogador**: O faxineiro possui temperatura corporal de **36.0 °C**, funcionando como fonte de calor por contato.
- **Base de Dados**: Os atributos são carregados dinamicamente de `elementos.csv` (118 elementos).

### Elementos Implementados e Previstos
1. **Gálio (Ga — Nº Atômico 31)**
   - **Estado a 20 °C**: Sólido | **Ponto de Fusão**: 29.8 °C.
   - **Reação ao Calor (Contato)**: Ao encostar no jogador (36.0 °C), absorve calor a uma taxa de +4.0 °C/s. Ao atingir ou ultrapassar 29.8 °C, sofre fusão e muda para o estado **Líquido**, transformando-se em uma poça que reduz a velocidade do jogador para 40% e a força do pulo para 70%.
   - **Reação à Água (Projétil do Mop)**: Fora de contato com o calor, resfria naturalmente a -2.0 °C/s até voltar a 20.0 °C. Quando atingido por água, recebe um efeito de resfriamento por 5 segundos (300 frames) que dobra a taxa de perda térmica (-4.0 °C/s), solidificando rapidamente. No estado **Sólido**, possui colisão superior rígida, servindo como plataforma para o jogador.
2. **Ferro (Fe — Nº Atômico 26)** *(Referência de catálogo / futuras fases)*
   - **Estado a 20 °C**: Sólido | **Ponto de Fusão**: 1538.0 °C | **Magnético**: Sim.
   - **Reação**: Imune ao calor corporal; exige fontes térmicas extremas ou interações magnéticas.

## 4. Estilo Visual e Técnico
- **Estética**: Pixel Art 2D side-scroller com temática industrial-espacial (texturas metálicas, tubulações, portas pressurizadas, saídas de emergência e iluminação de alerta).
- **Resolução e Câmera**: Resolução lógica base de 480x270 (com zoom de 240x135 na Sala 0), escalada para 1920x1080 a 60 FPS fixos.
- **Renderização**: Uso de sprites animados (`sprites/`) com Z-index condicional para objetos de cenário (`prop1.png`) e fallback automático em primitivas geométricas (retângulos coloridos) caso o sprite não seja encontrado.