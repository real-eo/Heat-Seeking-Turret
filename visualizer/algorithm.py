from data import *
from main import Heatmap

def cubic_interpolate(p, x):
    # * These coefficients are derived from the Hermite spline interpolation formula
    return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])))

def bicubic_interpolate(p, x, y):
    arr = [cubic_interpolate(p[i], y) for i in range(4)]
    return cubic_interpolate(arr, x)

def bicubic_interpolation(input_matrix, grid_size):
    scale_factor = 4    # Scaling factor to go from 8x8 to 128x128
    output_matrix = numpy.zeros((grid_size * scale_factor, grid_size * scale_factor))

    for i in range(grid_size * scale_factor):
        for j in range(grid_size * scale_factor):
            # * Find the x and y values on the input matrix corresponding to the current point (i, j) on the output matrix
            x = (i / float(scale_factor)) - 0.5
            y = (j / float(scale_factor)) - 0.5

            # * Get the integer and fractional parts of the x and y values
            x_int = int(numpy.floor(x))
            y_int = int(numpy.floor(y))
            x_frac = x - x_int
            y_frac = y - y_int

            # * Create a 4x4 grid of the nearest 16 points around the point (i, j)
            p = numpy.zeros((4, 4))
            for m in range(-1, 3):
                for n in range(-1, 3):
                    # * Ensure that we don't go out of bounds when accessing the input matrix
                    xm = min(max(x_int + m, 0), grid_size - 1)
                    yn = min(max(y_int + n, 0), grid_size - 1)

                    # * Get the values of the 16 from the input matrix and store them in the 4x4 grid 
                    p[m + 1, n + 1] = input_matrix[xm, yn]


            # * Interpolate the fractional part of the position (x, y) using bicubic interpolation,
            # * and add the interpolated value to the 4x4 grid surrounding the point (i, j) in the output matrix

            # print(p)
            output_matrix[i, j] = bicubic_interpolate(p, x_frac, y_frac)    

            # print(output_matrix[i, j])
            # print()

    return output_matrix

if __name__ == "__main__":
    app = Heatmap(data:=bicubic_interpolation(HEATMATRIX, 8))
    app.mainloop()
