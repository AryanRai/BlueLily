import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque

# Configuration
serial_port = "COM4"  # Update this for your system
baud_rate = 115200
buffer_size = 50  # Length of data points to display
read_batch_size = 5  # Number of serial lines to read per update

# Initialize plots
fig, ax = plt.subplots(2, 1, figsize=(10, 8))
ax[0].set_title("Accelerometer Data")
ax[1].set_title("Gyroscope Data")

accel_lines = [ax[0].plot([], [], label=label)[0] for label in ["AccelX", "AccelY", "AccelZ"]]
gyro_lines = [ax[1].plot([], [], label=label)[0] for label in ["GyroX", "GyroY", "GyroZ"]]

for axis in ax:
    axis.legend()
    axis.grid(True)

plt.tight_layout()

# Serial connection
ser = serial.Serial(serial_port, baud_rate, timeout=0.1)

# Data storage
accel_data = {key: deque([0] * buffer_size, maxlen=buffer_size) for key in ["AccelX", "AccelY", "AccelZ"]}
gyro_data = {key: deque([0] * buffer_size, maxlen=buffer_size) for key in ["GyroX", "GyroY", "GyroZ"]}


def update_plot(frame):
    global accel_data, gyro_data

    # Read multiple serial lines at once
    for _ in range(read_batch_size):
        try:
            line = ser.readline().decode().strip()
            if not line:
                continue

            values = line.split(",")
            if len(values) < 7:
                continue

            # Parse values for accelerometer and gyroscope
            ax_vals = [float(values[i]) for i in range(3)]
            gy_vals = [float(values[i + 3]) for i in range(3)]

            # Update deque buffers
            for i, key in enumerate(["AccelX", "AccelY", "AccelZ"]):
                accel_data[key].append(ax_vals[i])
            for i, key in enumerate(["GyroX", "GyroY", "GyroZ"]):
                gyro_data[key].append(gy_vals[i])

        except Exception as e:
            print(f"Read error: {e}")

    # Update plot lines
    for i, line in enumerate(accel_lines):
        line.set_data(range(buffer_size), list(accel_data[line.get_label()]))

    for i, line in enumerate(gyro_lines):
        line.set_data(range(buffer_size), list(gyro_data[line.get_label()]))

    # Adjust axis limits
    for axis in ax:
        axis.relim()
        axis.autoscale_view()


# Start animation
ani = FuncAnimation(fig, update_plot, interval=50)

plt.show()
