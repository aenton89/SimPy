import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_agg import FigureCanvasAgg
import base64
from io import BytesIO

PLOT_FUNCS = [
    # Funkcje rysujące
    "plot", "scatter", "bar", "barh", "hist",
    "imshow", "pcolormesh", "contour", "contourf",
    "specgram", "pie", "stackplot", "stem", "stairs",
    "errorbar", "fill", "fill_between", "violinplot", "boxplot",
    "hexbin", "quiver", "streamplot",

    # Elementy opisu
    "legend", "title", "xlabel", "ylabel", "xticks", "yticks",
    "xlim", "ylim", "text", "annotate", "grid", "axhline", "axvline",
    "axhspan", "axvspan", "suptitle", "tight_layout",

    # Zamykanie / odświeżanie
    "show", "savefig"
]


_original_funcs = {name: getattr(plt, name) for name in PLOT_FUNCS if hasattr(plt, name)}

def capture_and_call(name, *args, **kwargs):
    global __getplot__

    _original_funcs[name](*args, **kwargs)

    fig = plt.gcf()
    canvas = FigureCanvasAgg(fig)

    buffer = BytesIO()
    canvas.print_png(buffer)

    buffer.seek(0)
    base64_data = base64.b64encode(buffer.getvalue()).decode('utf-8')

    __getplot__ = base64_data


def make_wrapper(n):
    def wrapper(*a, **k):
        return capture_and_call(n, *a, **k)
    return wrapper

for name in _original_funcs:
    setattr(plt, name, make_wrapper(name))
