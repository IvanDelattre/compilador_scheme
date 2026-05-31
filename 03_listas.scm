; ===== Exemplo 3: Listas =====

; Criação de listas
(define minha-lista (list 1 2 3 4 5))

; Operações básicas
(display "Lista: ")
(display minha-lista)
(newline)

(display "Primeiro elemento (car): ")
(display (car minha-lista))
(newline)

(display "Restante (cdr): ")
(display (cdr minha-lista))
(newline)

; cons: adiciona elemento na frente
(define nova-lista (cons 0 minha-lista))
(display "Nova lista com cons: ")
(display nova-lista)
(newline)

; Função recursiva com listas: soma dos elementos
(define (soma-lista lst)
  (if (null? lst)
      0
      (+ (car lst) (soma-lista (cdr lst)))))

(display "Soma da lista: ")
(display (soma-lista minha-lista))
(newline)

; Comprimento de uma lista
(define (comprimento lst)
  (if (null? lst)
      0
      (+ 1 (comprimento (cdr lst)))))

(display "Comprimento: ")
(display (comprimento minha-lista))
(newline)
