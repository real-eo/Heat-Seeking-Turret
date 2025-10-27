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


def cubic_interpolate(p, x):
    # Perform cubic interpolation on a 1D array of 4 values
    # The coefficients are derived from the cubic Hermite spline interpolation formula
    return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])))

def bicubic_interpolate(p, x, y):
    # Perform cubic interpolation along the rows
    arr = [cubic_interpolate(p[i], y) for i in range(4)]
    # Perform cubic interpolation along the columns
    return cubic_interpolate(arr, x)

def bicubic_interpolation(input_matrix, grid_size):
    scale_factor = 2    # Scaling factor to go from 8x8 to 16x16
    output_matrix = [0 for _ in range(grid_size * scale_factor * grid_size * scale_factor)]

    for i in range(grid_size * scale_factor):
        for j in range(grid_size * scale_factor):
            # Find the x and y values on the input matrix corresponding to the current point (i, j) on the output matrix
            x = (i / float(scale_factor)) - 0.5
            y = (j / float(scale_factor)) - 0.5

            # Get the integer and fractional parts of the x and y values
            x_int = int(x)
            y_int = int(y)
            x_frac = x - x_int
            y_frac = y - y_int

            # Create a 4x4 grid of the nearest 16 points around the point (x_int, y_int)
            p = [[0 for _ in range(4)] for _ in range(4)]
            for m in range(-1, 3):
                for n in range(-1, 3):
                    # Ensure that we don't go out of bounds when accessing the input matrix
                    xm = min(max(x_int + m, 0), grid_size - 1)
                    yn = min(max(y_int + n, 0), grid_size - 1)

                    # Get the values of the 16 points from the input matrix and store them in the 4x4 grid 
                    p[m + 1][n + 1] = input_matrix[xm * grid_size + yn]

            # Interpolate the value at the fractional position (x_frac, y_frac) using bicubic interpolation,
            # and store the interpolated value in the output matrix at position (i, j)
            output_matrix[i * grid_size * scale_factor + j] = bicubic_interpolate(p, x_frac, y_frac)

    return output_matrix

def regional_bicubic_interpolation(input_matrix: list, grid_size):
    scale_factor = 2    # Scaling factor to go from 8x8 to 16x16
    output_matrix = [0 for _ in range(grid_size * scale_factor * grid_size * scale_factor)]
    
    # Information about the matrix
    max_value = max(input_matrix)
    max_index = input_matrix.index(max_value)
    max_x = max_index % 16
    max_y = max_index // 16
    offset = 16 >> 1

    # //for i in range(grid_size * scale_factor):
        # //for j in range(grid_size * scale_factor):
    # Find the x and y values on the input matrix corresponding to the current point (i, j) on the output matrix
    x = (max_x / float(scale_factor)) - 0.5
    y = (max_y / float(scale_factor)) - 0.5

    # Get the integer and fractional parts of the x and y values
    x_int = int(x)
    y_int = int(y)
    x_frac = x - x_int
    y_frac = y - y_int

    # Create a 4x4 grid of the nearest 16 points around the point (x_int, y_int)
    p = [[0 for _ in range(4)] for _ in range(4)]
    for m in range(-1, 3):
        for n in range(-1, 3):
            # Ensure that we don't go out of bounds when accessing the input matrix
            xm = min(max(x_int + m, 0), grid_size - 1)
            yn = min(max(y_int + n, 0), grid_size - 1)

            # Get the values of the 16 points from the input matrix and store them in the 4x4 grid 
            p[m + 1][n + 1] = input_matrix[xm * grid_size + yn]

    # Interpolate the value at the fractional position (x_frac, y_frac) using bicubic interpolation,
    # and store the interpolated value in the output matrix at position (i, j)
    output_matrix[max_x * grid_size * scale_factor + max_y] = bicubic_interpolate(p, x_frac, y_frac)

    return output_matrix


if __name__ == "__main__":
    matrix = bicubic_interpolation(HEATMATRIX, 8)
    # matrix = regional_bicubic_interpolation(HEATMATRIX, 8)

    # ONLY turn the matrix into a 2D array AFTER interpolation
    import numpy
    
    array = numpy.array(matrix).reshape((16, 16))
    app = Heatmap(array, (max(matrix), min(matrix)))
    
    app.mainloop()
