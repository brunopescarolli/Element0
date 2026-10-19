# Jogo educativo de reações químicas (C + Allegro 5)

## Regras fixas
- Linguagem: C, biblioteca Allegro 5.
- Compilar com: gcc main.c -o jogo -static-libgcc -static-libstdc++ $(pkg-config --cflags --libs allegro-5 allegro_main-5 allegro_primitives-5 allegro_image-5 allegro_font-5)
- Nunca diga que terminou sem compilar sem warnings.
- Mantenha todos os sprites existentes em assets/sprites. Use retângulos coloridos apenas como fallback quando o bitmap for NULL.
- Os caminhos de arquivos (sprites/ e elementos.csv) estão dentro de assets.
- Fases são arquivos em assets/fases/, não código.
- Reações ficam numa tabela de regras (elemento + condição -> efeito).
- Antes de agir, leia docs/design.md, docs/fases/ e TASKS.md.
- Ao terminar uma tarefa: marque em TASKS.md e faça commit.
- Não reescreva arquivos inteiros sem necessidade; faça mudanças pontuais.