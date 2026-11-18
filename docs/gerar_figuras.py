"""Gera as figuras da documentação: diagrama da lista encadeada e gráfico de crescimento das ordenações.
Uso (na raiz do projeto): python docs/gerar_figuras.py   (lê tests/resultado_testes.txt)"""
import re
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, FancyBboxPatch

RAIZ = Path(__file__).resolve().parent.parent
AZUL, VERDE, CINZA, VERMELHO = "#1F4E79", "#2E7D32", "#5F6B76", "#C62828"
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 10})


def no(ax, x, y, id_, titulo, destaque=None):
    cor = destaque or AZUL
    ax.add_patch(FancyBboxPatch((x, y), 2.2, 1.1, boxstyle="round,pad=0.02,rounding_size=0.08",
                                fc="white", ec=cor, lw=1.6))
    ax.plot([x + 1.65, x + 1.65], [y, y + 1.1], color=cor, lw=1.2)
    ax.text(x + 0.82, y + 0.72, f"id {id_}", ha="center", va="center", fontsize=9.5, weight="bold", color=cor)
    ax.text(x + 0.82, y + 0.34, titulo, ha="center", va="center", fontsize=7.5, color=CINZA)
    ax.text(x + 1.92, y + 0.55, "prox", ha="center", va="center", fontsize=7, color=cor, rotation=90)


def seta(ax, x0, y0, x1, y1, cor=AZUL, estilo="-|>", ls="-", rad=0.0):
    ax.add_patch(FancyArrowPatch((x0, y0), (x1, y1), arrowstyle=estilo, mutation_scale=12, color=cor,
                                 lw=1.4, linestyle=ls, connectionstyle=f"arc3,rad={rad}"))


def diagrama_lista():
    fig, ax = plt.subplots(figsize=(10, 4.6))
    ax.set_xlim(0, 16)
    ax.set_ylim(-0.4, 7.4)
    ax.axis("off")
    # lista normal
    ax.text(0, 6.95, "Lista com 4 nós (cada nó alocado com malloc)", fontsize=10.5, weight="bold", color=AZUL)
    xs = [1.6, 5.0, 8.4, 11.8]
    dados = [(10, "Bilhete Único"), (3, "Gratuidades"), (7, "Tarifas"), (21, "Energia")]
    for x, (i, t) in zip(xs, dados):
        no(ax, x, 4.9, i, t)
    for a, b in zip(xs, xs[1:]):
        seta(ax, a + 1.92, 5.45, b - 0.02, 5.45)
    seta(ax, xs[-1] + 1.92, 5.45, 15.2, 5.45)
    ax.text(15.25, 5.45, "NULL", va="center", fontsize=9, color=CINZA)
    ax.text(0.05, 6.25, "inicio", fontsize=9, color=VERDE, weight="bold")
    seta(ax, 0.7, 6.15, xs[0] + 0.2, 6.02, cor=VERDE)
    ax.text(13.1, 6.45, "fim", fontsize=9, color=VERDE, weight="bold")
    seta(ax, 13.3, 6.35, xs[-1] + 1.0, 6.02, cor=VERDE)
    ax.text(0.05, 4.55, "tamanho = 4", fontsize=8.5, color=CINZA)
    # remoção do meio
    ax.text(0, 3.35, "Remoção do id 3: anterior->prox passa a apontar para atual->prox; depois free(atual)",
            fontsize=10.5, weight="bold", color=AZUL)
    for x, (i, t) in zip(xs, dados):
        no(ax, x, 1.2, i, t, destaque=VERMELHO if i == 3 else None)
    seta(ax, xs[0] + 1.92, 1.95, xs[2] - 0.02, 1.95, cor=VERDE, rad=-0.35)
    seta(ax, xs[1] + 1.92, 1.75, xs[2] - 0.02, 1.75, cor=VERMELHO, ls="--")
    seta(ax, xs[0] + 1.92, 1.55, xs[1] - 0.02, 1.55, cor=VERMELHO, ls="--")
    seta(ax, xs[2] + 1.92, 1.75, xs[3] - 0.02, 1.75)
    seta(ax, xs[-1] + 1.92, 1.75, 15.2, 1.75)
    ax.text(15.25, 1.75, "NULL", va="center", fontsize=9, color=CINZA)
    ax.text(xs[1] + 1.1, 0.75, "free(atual)", ha="center", fontsize=8.5, color=VERMELHO)
    ax.text(xs[0] + 3.0, 3.0, "anterior", ha="center", fontsize=8.5, color=VERDE)
    fig.savefig(RAIZ / "docs" / "diagrama_lista.png", dpi=200, bbox_inches="tight", facecolor="white")
    plt.close(fig)


def grafico_crescimento():
    txt = (RAIZ / "tests" / "resultado_testes.txt").read_text(encoding="utf-8")
    linhas = re.findall(r"^\s*(\d+) \|\s+(\d+) \|\s+(\d+) \|\s+(\d+) \|\s+(\d+)", txt, re.M)
    n = [int(l[0]) for l in linhas]
    merge = [int(l[1]) for l in linhas]
    ref_m = [int(l[2]) for l in linhas]
    ins = [int(l[3]) for l in linhas]
    ref_i = [int(l[4]) for l in linhas]
    fig, ax = plt.subplots(figsize=(8, 4.2))
    ax.plot(n, ins, "o-", color=VERMELHO, label="Insertion Sort (medido)")
    ax.plot(n, ref_i, "--", color=VERMELHO, alpha=0.5, label="n²/4 (caso médio teórico)")
    ax.plot(n, merge, "o-", color=AZUL, label="Merge Sort (medido)")
    ax.plot(n, ref_m, "--", color=AZUL, alpha=0.5, label="n·⌈log₂ n⌉ (teórico)")
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlabel("n (registros, ordem aleatória)")
    ax.set_ylabel("comparações (escala log)")
    ax.set_xticks(n)
    ax.set_xticklabels([str(v) for v in n])
    ax.grid(True, which="major", alpha=0.3)
    ax.legend(frameon=False, fontsize=8.5)
    ax.set_title("Comparações feitas pelas ordenações na lista encadeada", fontsize=11)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    fig.tight_layout()
    fig.savefig(RAIZ / "docs" / "crescimento_ordenacao.png", dpi=200, facecolor="white")
    plt.close(fig)


if __name__ == "__main__":
    diagrama_lista()
    grafico_crescimento()
    print("Figuras geradas em docs/")
