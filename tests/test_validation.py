"""Entradas numericas invalidas devem falhar sem produzir uma grade."""
from pathlib import Path
import subprocess
import sys


def main():
    binary = Path(sys.argv[1]).resolve()
    cases = {
        'linhas zero': b'0 1 0\n1\n',
        'linhas negativas': b'-1 1 0\n1\n',
        'colunas zero': b'1 0 0\n1\n',
        'colunas negativas': b'1 -1 0\n1\n',
        'geracoes negativas': b'1 1 -1\n1\n',
        'sem regras': b'1 1 0\n0\n',
        'regras negativas': b'1 1 0\n-1\n',
        'inteiro enorme': b'999999999999999999999999999999 1 0\n1\n',
        'token longo': b'9' * 100 + b' 1 0\n1\n',
        'token parcial': b'1abc 1 0\n1\n',
        'cabecalho incompleto': b'1 1\n',
        'id enorme': b'1 1 0\n1\nB3/S23\n999999999999999999999999\nx\n',
        'id negativo': b'1 1 0\n1\nB3/S23\n-1\nx\n',
        'id fora da tabela': b'1 1 0\n1\nB3/S23\n1\nx\n',
        'linha curta': b'1 3 0\n1\nB3/S23\n0 0 0\nx \n',
        'linha vazia': b'1 3 0\n1\nB3/S23\n0 0 0\n\n',
        'linha longa': b'1 1 0\n1\nB3/S23\n0\nxx\n',
        'espaco excedente': b'1 1 0\n1\nB3/S23\n0\nx \n',
        'segunda linha curta': b'2 3 0\n1\nB3/S23\n0 0 0\n0 0 0\nxxx\nx\n',
        'EOF com linha curta': b'1 3 0\n1\nB3/S23\n0 0 0\nx',
        'CR isolado': b'1 1 0\n1\nB3/S23\n0\nx\r',
        'grade ausente': b'1 1 0\n1\nB3/S23\n0\n',
    }
    for name, data in cases.items():
        result = subprocess.run([str(binary)], input=data, capture_output=True, timeout=5)
        assert result.returncode == 1, (name, result.returncode, result.stderr)
        assert result.stdout == b'', (name, result.stdout)
        assert result.stderr, name
        assert b'Sanitizer' not in result.stderr, (name, result.stderr)
    print(f'OK: {len(cases)} entradas invalidas rejeitadas com diagnostico.')
    header = b'1 3 0\n1\nB3/S23\n0 0 0\n'
    valid = [header + b'x  \n', header + b'x  ',
             (header + b'x  \n').replace(b'\n', b'\r\n')]
    for data in valid:
        result = subprocess.run([str(binary)], input=data, capture_output=True, timeout=5)
        assert result.returncode == 0 and result.stdout == b'x  \n', result
        assert result.stderr == b'', result.stderr
    print('OK: LF, CRLF e ultima linha sem quebra preservam espacos e G=0.')


if __name__ == '__main__':
    main()
