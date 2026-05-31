; ===== Exemplo 4: Let e Lambda =====

; let: variáveis locais
(define (area-retangulo b h)
  (let ((base b)
        (altura h))
    (* base altura)))

(display "Área do retângulo (4x5): ")
(display (area-retangulo 4 5))
(newline)

; let*: bindings sequenciais (cada um pode usar o anterior)
(define (hipotenusa a b)
  (let* ((a2 (* a a))
         (b2 (* b b))
         (soma (+ a2 b2)))
    soma))

(display "Quadrado da hipotenusa (3,4): ")
(display (hipotenusa 3 4))
(newline)

; lambda: funções anônimas
(define dobro (lambda (x) (* x 2)))
(define triplo (lambda (x) (* x 3)))

(display "Dobro de 7: ")
(display (dobro 7))
(newline)

(display "Triplo de 7: ")
(display (triplo 7))
(newline)

; Funções de alta ordem
(define (aplicar f x)
  (f x))

(display "Aplicar dobro em 10: ")
(display (aplicar dobro 10))
(newline)
