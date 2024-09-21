import numpy as np
from scipy.optimize import least_squares

def circle_residuals(params, x, y):
    xc, yc, r = params
    return (x - xc)**2 + (y - yc)**2 - r**2

def fit_circle(x, y):
    x_m = np.mean(x)
    y_m = np.mean(y)
    initial_guess = (x_m, y_m, np.mean(np.sqrt((x - x_m)**2 + (y - y_m)**2)))
    result = least_squares(circle_residuals, initial_guess, args=(x, y))
    return result.x

def find_rotation_center(corner_trajectories):
    rc_list = [fit_circle(corner[:, 0], corner[:, 1])[:2] for corner in corner_trajectories]
    rc = np.mean(rc_list, axis=0)
    return rc

def find_cg(corners_initial, rc):
    corners_mean = np.mean(corners_initial, axis=0)
    cg = corners_mean + (corners_mean - rc)
    return cg

def transform_to_body_fixed(corners, cg, theta):
    corners_relative = corners - cg
    rotation_matrix = np.array([
        [np.cos(theta), -np.sin(theta)],
        [np.sin(theta), np.cos(theta)]
    ])
    corners_body_fixed = np.dot(rotation_matrix, corners_relative.T).T
    return corners_body_fixed

def load_data(file_path):
    data = np.loadtxt(file_path)
    time = data[:, 0]
    corner_trajectories = [
        data[:, 1:4],  # Corner 1 trajectory (x, y, z)
        data[:, 4:7],  # Corner 2 trajectory (x, y, z)
        data[:, 7:10],  # Corner 3 trajectory (x, y, z)
        data[:, 10:13]  # Corner 4 trajectory (x, y, z)
    ]
    return time, corner_trajectories

# Load data from file
file_path = 'auto_calibrate_servicer2.txt'  # Replace with the actual path to your file
time, corner_trajectories = load_data(file_path)

# Find the rotation center (RC)
rc = find_rotation_center(corner_trajectories)

# Example initial positions of the corners (when the square is aligned with the coordinate axes)
corners_initial = np.array([
    [corner_trajectories[0][0, 0], corner_trajectories[0][0, 1]],
    [corner_trajectories[1][0, 0], corner_trajectories[1][0, 1]],
    [corner_trajectories[2][0, 0], corner_trajectories[2][0, 1]],
    [corner_trajectories[3][0, 0], corner_trajectories[3][0, 1]]
])

# Calculate the center of geometry (GC)
gc = np.mean(corners_initial, axis=0)

# Find the center of gravity (CG)
cg = find_cg(corners_initial, rc)

# Calculate the offset of CG from GC
cg_offset = cg - gc

# Example rotation angle (in radians)
theta = np.pi / 4  # 45 degrees

# Transform corners to the body-fixed frame
corners_body_fixed = transform_to_body_fixed(corners_initial, cg, theta)

# Output the results
print("Rotation Center (RC):", rc)
print("Center of Gravity (CG):", cg)
print("Corners in Body-Fixed Frame:\n", corners_body_fixed)
