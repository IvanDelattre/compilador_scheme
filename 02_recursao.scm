; ===== Exemplo 2: Recursão =====

; Fatorial
(define (fatorial n)
  (if (= n 0)
      1
      (* n (fatorial (- n 1)))))

; Fibonacci
(define (fibonacci n)
  (cond ((= n 0) 0)
        ((= n 1) 1)
        (else (+ (fibonacci (- n 1))
                 (fibonacci (- n 2))))))

; Testes
(display "Fatorial de 5: ")
(display (fatorial 5))
(newline)

(display "Fatorial de 10: ")
(display (fatorial 10))
(newline)

(display "Fibonacci de 10: ")
(display (fibonacci 10))
(newline)
