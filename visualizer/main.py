try: from data import *
except: from .data import *

import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg


def plot_heatmap(matrix, minValue, maxValue):
    fig, ax = plt.subplots()
    cax = ax.matshow(matrix, cmap='hot', vmin=minValue, vmax=maxValue)
    fig.colorbar(cax)
    return fig

class Heatmap(tk.Tk):
    def __init__(self, matrix, valueRange=VALUE_RANGE):
        super().__init__()
        self.title("Heatmap Visualizer")
        self.geometry("600x600")
        
        fig = plot_heatmap(matrix, *valueRange)
        canvas = FigureCanvasTkAgg(fig, master=self)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

if __name__ == "__main__":
    app = Heatmap(HEATMATRIX)
    app.mainloop()