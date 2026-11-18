"""Converte a base real do grupo (data/base_conhecimento.json, repositório citizen-assist-ai)
para o formato texto lido pelo programa em C: id|categoria|titulo|descricao|requisitos|local|fonte|verificado_em

Uso: python ferramentas/converter_base.py caminho/base_conhecimento.json dados/orientacoes.txt
"""
import json
import sys

LIMITES = {"categoria": 47, "titulo": 119, "descricao": 1023, "requisitos": 511, "local": 199, "fonte": 199}


def limpar(texto):
    return " ".join(str(texto).replace("|", "/").split())


def main(origem, destino):
    with open(origem, encoding="utf-8") as f:
        base = json.load(f)
    linhas = ["# Citizen Assist AI Mobilidade - base de orientações (UTF-8)",
              "# Origem: base_conhecimento.json do repositório FIAP-GS-02-2026/citizen-assist-ai",
              "# id|categoria|titulo|descricao|requisitos|local|fonte|verificado_em"]
    for r in base:
        campos = {
            "categoria": limpar(r["categoria"]),
            "titulo": limpar(r["titulo"]),
            "descricao": limpar(r["resposta"]),
            "requisitos": limpar("; ".join(r.get("documentos", []))),
            "local": limpar(r["onde"]),
            "fonte": limpar(r["fonte"]),
        }
        for nome, limite in LIMITES.items():
            if len(campos[nome].encode("utf-8")) > limite:
                raise SystemExit(f"Registro {r['id']}: campo {nome} passa do limite de {limite} bytes")
        linhas.append("|".join([str(r["id"]), campos["categoria"], campos["titulo"], campos["descricao"],
                                campos["requisitos"], campos["local"], campos["fonte"], r["verificado_em"]]))
    with open(destino, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(linhas) + "\n")
    print(f"{len(base)} orientações gravadas em {destino}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
