from main import Heatmap

HEATMATRIX = [
    1750, 1775, 1775, 1725, 1700, 1825, 1875, 1825, 
    1775, 1800, 1725, 1775, 1825, 1825, 1850, 1900, 
    1800, 1750, 1700, 1775, 1800, 1800, 1925, 2050, 
    1800, 1825, 1750, 1775, 1925, 2075, 2100, 2225, 
    1875, 1875, 1850, 1850, 2225, 2150, 2275, 2325, 
    1950, 2075, 2075, 2150, 2175, 2075, 2200, 2375, 
    2125, 2250, 2425, 2350, 2075, 2000, 2150, 2250, 
    2250, 2175, 2225, 2150, 2150, 2150, 2150, 2175, 
]

def bilinearInterpolate(p, x, y) -> float:
    return p[0] * (1 - x) * (1 - y) + p[1] * x * (1 - y) + p[2] * (1 - x) * y + p[3] * x * y


def bilinearInterpolation(matrix, gridSize) -> list:
    scale_factor = 2
    output_matrix = []

    for i in range(gridSize * scale_factor):
        for j in range(gridSize * scale_factor):
            x = (i / float(scale_factor)) - 0.5
            y = (j / float(scale_factor)) - 0.5
            x_int = int(x)
            y_int = int(y)
            x_frac = x - x_int
            y_frac = y - y_int

            p = []
            for m in range(-1, 2):
                for n in range(-1, 2):
                    xm = min(max(x_int + m, 0), gridSize - 1)
                    yn = min(max(y_int + n, 0), gridSize - 1)
                    p.append(matrix[xm * gridSize + yn])

            output_matrix.append(bilinearInterpolate(p, x_frac, y_frac))

    return output_matrix


if __name__ == "__main__":
    matrix = bilinearInterpolation(HEATMATRIX, 8)

    # ONLY turn the matrix into a 2D array AFTER interpolation
    import numpy
    
    array = numpy.array(matrix).reshape((16, 16))
    app = Heatmap(array, (max(matrix), min(matrix)))
    
    app.mainloop()
