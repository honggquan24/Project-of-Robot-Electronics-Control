import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import serial
import serial.tools.list_ports
import threading
import time
import queue
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from scipy.optimize import curve_fit
import csv

# --- Global Variables ---
ser = None
data_queue = queue.Queue()
serial_thread_running = False
calibration_points = []  # List to store (distance, adc) tuples
MAX_PLOT_POINTS = 100  # For real-time ADC plotting (optional)
adc_plot_data = []


# --- Các hàm khớp mẫu (giống như trước) ---
def inverse_func(adc, a, b, c):
    return a / (np.maximum(adc, 1e-9) - b) + c


def power_func(adc, a, b):
    return a * np.power(np.maximum(adc, 1e-9), b)


def simple_inv_adc_offset(adc, a, b):
    return a / np.maximum(adc, 1e-9) + b


# Thêm các hàm khác nếu muốn


# --- Serial Communication ---
# (Giữ nguyên các hàm list_serial_ports, disconnect_serial, send_serial_command từ GUI trước)
# Sửa đổi connect_serial và read_from_serial
def list_serial_ports_gui():
    ports = [port.device for port in serial.tools.list_ports.comports()]
    port_combobox["values"] = ports
    if ports:
        port_combobox.current(0)


def connect_serial_gui():
    global ser, serial_thread_running
    if ser and ser.is_open:  # Nếu đang kết nối thì ngắt kết nối
        disconnect_serial_gui()
        return

    port = port_combobox.get()
    if not port:
        messagebox.showerror("Error", "Please select a serial port.")
        return

    baudrate = 115200
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(1)  # Chờ ESP32

        ser.reset_input_buffer()  # Xóa buffer cũ
        # Chờ tín hiệu ESP_CALIB_READY (hoặc tương tự)
        ready_signal = ""
        start_time = time.time()
        while time.time() - start_time < 3:  # Timeout 3 giây
            if ser.in_waiting > 0:
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                if "ESP_CALIB_READY" in line:
                    ready_signal = line
                    break
            time.sleep(0.1)

        if ser.is_open:
            status_label.config(text=f"Connected to {port}", foreground="green")
            connect_button.config(text="Disconnect")
            serial_thread_running = True
            threading.Thread(target=read_from_serial_gui, daemon=True).start()
            record_button.config(state=tk.NORMAL)
            fit_button.config(state=tk.NORMAL)
            save_button.config(state=tk.NORMAL)
            clear_button.config(state=tk.NORMAL)
        else:
            ser.close()
            ser = None
            status_label.config(text="ESP32 Calibrator not ready.", foreground="red")
            messagebox.showerror(
                "Connection Error", "ESP32 Calibrator not ready or wrong port."
            )
    except serial.SerialException as e:
        status_label.config(text=f"Error: {e}", foreground="red")
        ser = None
        messagebox.showerror("Connection Error", f"Could not open port {port}: {e}")


def disconnect_serial_gui():
    global ser, serial_thread_running
    serial_thread_running = False
    time.sleep(0.1)  # Cho thread thời gian dừng
    if ser and ser.is_open:
        ser.close()
    status_label.config(text="Disconnected", foreground="black")
    connect_button.config(text="Connect")
    record_button.config(state=tk.DISABLED)
    fit_button.config(state=tk.DISABLED)
    save_button.config(state=tk.DISABLED)
    clear_button.config(state=tk.DISABLED)
    ser = None


def send_serial_command_gui(command):
    if ser and ser.is_open:
        try:
            ser.write((command + "\n").encode("utf-8"))
        except Exception as e:
            print(f"Error sending command: {e}")
    else:
        if (
            command.get() != ""
        ):  # Chỉ hiện warning nếu người dùng cố gửi lệnh khi chưa kết nối
            messagebox.showwarning("Serial Error", "Not connected to ESP32.")


def read_from_serial_gui():
    global serial_thread_running
    while serial_thread_running:
        if ser and ser.is_open:
            try:
                if ser.in_waiting > 0:
                    line = ser.readline().decode("utf-8", errors="ignore").strip()
                    if line:
                        data_queue.put(line)  # Đưa dữ liệu vào queue để GUI xử lý
            except serial.SerialException:
                if (
                    serial_thread_running
                ):  # Chỉ xử lý nếu không phải do ngắt kết nối chủ động
                    print("Serial port error or disconnected during read.")
                    # Tự động thử ngắt kết nối trên GUI
                    root.after(0, disconnect_serial_gui)
                break
            except Exception as e:
                # print(f"Error reading serial: {e}") # Bỏ comment để debug
                pass
        else:
            break
        time.sleep(0.01)


# --- GUI Logic ---
def request_calibration_data():
    global calibration_points
    try:
        distance_cm_str = distance_entry.get()
        if not distance_cm_str:
            messagebox.showerror("Input Error", "Please enter a distance value.")
            return
        distance_cm = float(distance_cm_str)
        if distance_cm <= 0:
            messagebox.showerror("Input Error", "Distance must be positive.")
            return

        send_serial_command_gui(f"CALIB_REQUEST:{distance_cm_str}")
        status_label.config(
            text=f"Requested ADC for {distance_cm_str} cm...", foreground="blue"
        )
    except ValueError:
        messagebox.showerror(
            "Input Error", "Invalid distance format. Please enter a number."
        )


def update_gui_from_queue_calib():
    try:
        while not data_queue.empty():
            line = data_queue.get_nowait()
            # print(f"GUI RX: {line}") # Debug
            if line.startswith("CALIB_DATA:"):
                try:
                    # SỬA Ở ĐÂY: Dùng slicing thay vì substring
                    # Bỏ "CALIB_DATA:" (11 ký tự)
                    data_part = line[11:]  # Python slicing: lấy từ ký tự thứ 11 đến hết

                    parts = data_part.split(",")
                    if len(parts) == 2:
                        dist = float(parts[0])
                        adc = int(parts[1])
                        calibration_points.append((dist, adc))
                        update_calibration_table()
                        status_label.config(
                            text=f"Recorded: ({dist:.1f}cm, ADC {adc})",
                            foreground="green",
                        )
                        distance_entry.delete(0, tk.END)
                    else:
                        print(f"Malformed CALIB_DATA (not 2 parts after split): {line}")
                        status_label.config(
                            text="Error: Malformed CALIB_DATA from ESP32",
                            foreground="red",
                        )
                except Exception as e:
                    print(f"Error processing CALIB_DATA: {e} - Line: {line}")
                    status_label.config(
                        text=f"Error parsing CALIB_DATA: {e}", foreground="red"
                    )
            elif line.startswith("CALIB_ERROR:"):
                # SỬA Ở ĐÂY: Dùng slicing
                error_message = line[12:]  # Bỏ "CALIB_ERROR:"
                messagebox.showerror("ESP32 Error", error_message)
                status_label.config(
                    text=f"ESP32 Error: {error_message}", foreground="red"
                )
            elif "ESP_CALIB_READY" not in line:  # Bỏ qua tín hiệu ready ban đầu
                raw_serial_text.insert(tk.END, line + "\n")
                raw_serial_text.see(tk.END)
            # else: # In ra các dòng không khớp để debug nếu cần
            #     print(f"GUI RX Unhandled: {line}")

    except queue.Empty:
        pass  # No new data
    root.after(100, update_gui_from_queue_calib)


def update_calibration_table():
    # Xóa bảng cũ
    for widget in table_frame.winfo_children():
        widget.destroy()

    # Tạo header
    ttk.Label(table_frame, text="Distance (cm)", font=("Arial", 10, "bold")).grid(
        row=0, column=0, padx=5, pady=2
    )
    ttk.Label(table_frame, text="ADC Value", font=("Arial", 10, "bold")).grid(
        row=0, column=1, padx=5, pady=2
    )

    # Điền dữ liệu
    for i, (dist, adc) in enumerate(calibration_points):
        ttk.Label(table_frame, text=f"{dist:.1f}").grid(row=i + 1, column=0, padx=5)
        ttk.Label(table_frame, text=str(adc)).grid(row=i + 1, column=1, padx=5)

    # Cập nhật đồ thị nếu có đủ điểm
    if len(calibration_points) >= 2:
        plot_calibration_data()


def plot_calibration_data(fitted_params=None, func_to_plot=None, func_name=""):
    ax.clear()
    if calibration_points:
        distances = [p[0] for p in calibration_points]
        adcs = [p[1] for p in calibration_points]
        ax.scatter(
            adcs, distances, label="Dữ liệu hiệu chuẩn", color="blue", marker="o"
        )

        if fitted_params is not None and func_to_plot is not None:
            adc_line = np.linspace(min(adcs) * 0.9, max(adcs) * 1.1, 200)
            try:
                dist_line = func_to_plot(adc_line, *fitted_params)
                ax.plot(
                    adc_line, dist_line, label=f"Hàm khớp: {func_name}", color="red"
                )
            except Exception as e:
                print(f"Lỗi khi vẽ đường cong khớp: {e}")

    ax.set_xlabel("Giá trị ADC")
    ax.set_ylabel("Khoảng cách (cm)")
    ax.set_title("Đồ thị Hiệu chuẩn Cảm biến")
    ax.legend()
    ax.grid(True)
    if calibration_points:  # Chỉ đảo trục nếu có dữ liệu
        ax.invert_xaxis()
    canvas.draw()


def fit_curve_and_display():
    if len(calibration_points) < 3:  # Cần ít nhất số điểm bằng số tham số + 1
        messagebox.showwarning(
            "Warning", "Cần ít nhất 3 điểm dữ liệu để thực hiện curve fitting."
        )
        return

    distances = np.array([p[0] for p in calibration_points])
    adcs = np.array([p[1] for p in calibration_points])

    # --- CHỌN HÀM Ở ĐÂY ---
    # func_to_fit, func_name_str, initial_p = (inverse_func, "A/(adc-B)+C", [20000, 100, 0])
    func_to_fit, func_name_str, initial_p = (
        simple_inv_adc_offset,
        "A/adc + B",
        [60000, -15],
    )
    # func_to_fit, func_name_str, initial_p = (power_func, "A*adc^B", [100000, -1.0])

    try:
        params, covariance = curve_fit(
            func_to_fit, adcs, distances, p0=initial_p, maxfev=100000
        )

        result_text.set(f"Hàm khớp: {func_name_str}\n")
        param_names_map = {
            inverse_func: ["A", "B", "C"],
            power_func: ["A", "B"],
            simple_inv_adc_offset: ["A", "B"],
        }
        param_names = param_names_map.get(
            func_to_fit, [f"p{i+1}" for i in range(len(params))]
        )

        for name, val in zip(param_names, params):
            result_text.set(result_text.get() + f"  {name}: {val:.4f}\n")

        # Tạo code C++
        cpp_code = "float distance_cm = 0.0f;\n"
        cpp_code += "if (adcValue > 0) { // Tránh chia cho 0\n"
        if func_to_fit == inverse_func:
            cpp_code += f"    distance_cm = {params[0]:.4f}f / ( (float)adcValue - ({params[1]:.4f}f) ) + ({params[2]:.4f}f);\n"
        elif func_to_fit == power_func:
            cpp_code += f"    distance_cm = {params[0]:.4f}f * pow((float)adcValue, {params[1]:.4f}f);\n"
        elif func_to_fit == simple_inv_adc_offset:
            cpp_code += f"    distance_cm = {params[0]:.2f}f / (float)adcValue + ({params[1]:.2f}f);\n"
        # Thêm các hàm khác nếu bạn dùng
        cpp_code += (
            "} else {\n    distance_cm = 150.0f; // Giá trị mặc định nếu ADC=0\n}\n"
        )
        cpp_code += (
            "// Nhớ thêm: distance_cm = constrain(distance_cm, 20.0f, 150.0f);\n"
        )

        result_text.set(result_text.get() + "\nCode C++ gợi ý:\n" + cpp_code)

        plot_calibration_data(
            params, func_to_fit, func_name_str
        )  # Vẽ lại đồ thị với đường khớp

    except RuntimeError:
        messagebox.showerror(
            "Fit Error",
            "Không thể tìm thấy hàm khớp. Hãy thử các giá trị khởi tạo khác hoặc nhiều điểm dữ liệu hơn.",
        )
        result_text.set("Lỗi khi khớp hàm.")
    except Exception as e:
        messagebox.showerror("Error", f"Lỗi không xác định: {e}")
        result_text.set(f"Lỗi: {e}")


def save_calibration_data():
    if not calibration_points:
        messagebox.showinfo("Info", "Không có dữ liệu để lưu.")
        return
    filepath = filedialog.asksaveasfilename(
        defaultextension=".csv",
        filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        title="Lưu dữ liệu hiệu chuẩn",
    )
    if filepath:
        try:
            with open(filepath, "w", newline="") as f:
                writer = csv.writer(f)
                writer.writerow(["Distance", "ADC"])  # Header
                for dist, adc in calibration_points:
                    writer.writerow([dist, adc])
            messagebox.showinfo("Success", f"Dữ liệu đã được lưu vào:\n{filepath}")
        except Exception as e:
            messagebox.showerror("Save Error", f"Không thể lưu file: {e}")


def load_calibration_data():
    global calibration_points
    filepath = filedialog.askopenfilename(
        defaultextension=".csv",
        filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        title="Tải dữ liệu hiệu chuẩn",
    )
    if filepath:
        try:
            data = pd.read_csv(filepath)
            if "Distance" in data.columns and "ADC" in data.columns:
                calibration_points = list(zip(data["Distance"], data["ADC"]))
                update_calibration_table()
                plot_calibration_data()  # Vẽ lại đồ thị với dữ liệu đã tải
                messagebox.showinfo("Success", f"Dữ liệu đã được tải từ:\n{filepath}")
            else:
                messagebox.showerror(
                    "Load Error", "File CSV không có cột 'Distance' và 'ADC'."
                )
        except Exception as e:
            messagebox.showerror("Load Error", f"Không thể tải file: {e}")


def clear_calibration_data():
    global calibration_points
    if messagebox.askyesno(
        "Confirm", "Bạn có chắc muốn xóa tất cả dữ liệu hiệu chuẩn?"
    ):
        calibration_points = []
        update_calibration_table()
        plot_calibration_data()  # Xóa đồ thị
        result_text.set("Nhập các tham số và nhấn 'Fit Curve'.")


# --- GUI Setup ---
root = tk.Tk()
root.title("ESP32 Sensor Calibration Tool")

# Frame chính
main_frame = ttk.Frame(root, padding="10")
main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)

# --- Cột Trái: Điều khiển và Nhập liệu ---
left_column_frame = ttk.Frame(main_frame, padding="5")
left_column_frame.grid(row=0, column=0, sticky=(tk.N, tk.S, tk.W), padx=5)

# Connection
conn_frame = ttk.LabelFrame(left_column_frame, text="Serial Connection")
conn_frame.pack(fill=tk.X, pady=5)
port_label = ttk.Label(conn_frame, text="Port:")
port_label.pack(side=tk.LEFT, padx=2)
port_combobox = ttk.Combobox(conn_frame, width=12)
port_combobox.pack(side=tk.LEFT, padx=2)
refresh_ports_button = ttk.Button(
    conn_frame, text="Refresh", command=list_serial_ports_gui, width=8
)
refresh_ports_button.pack(side=tk.LEFT, padx=2)
connect_button = ttk.Button(
    conn_frame, text="Connect", command=connect_serial_gui, width=10
)
connect_button.pack(side=tk.LEFT, padx=2)
status_label = ttk.Label(conn_frame, text="Disconnected", width=25)
status_label.pack(side=tk.LEFT, padx=2, fill=tk.X, expand=True)
list_serial_ports_gui()  # Tải danh sách cổng ban đầu

# Data Input
input_frame = ttk.LabelFrame(left_column_frame, text="Record Calibration Point")
input_frame.pack(fill=tk.X, pady=10)
ttk.Label(input_frame, text="Distance (cm):").grid(
    row=0, column=0, padx=5, pady=5, sticky=tk.W
)
distance_entry = ttk.Entry(input_frame, width=10)
distance_entry.grid(row=0, column=1, padx=5, pady=5)
record_button = ttk.Button(
    input_frame, text="Record ADC", command=request_calibration_data, state=tk.DISABLED
)
record_button.grid(row=0, column=2, padx=5, pady=5)

# Calibration Table
table_outer_frame = ttk.LabelFrame(left_column_frame, text="Calibration Data Points")
table_outer_frame.pack(fill=tk.BOTH, expand=True, pady=5)
# Tạo canvas và scrollbar cho bảng
table_canvas = tk.Canvas(table_outer_frame)
table_scrollbar = ttk.Scrollbar(
    table_outer_frame, orient="vertical", command=table_canvas.yview
)
table_frame = ttk.Frame(table_canvas)  # Frame chứa các label dữ liệu
table_frame.bind(
    "<Configure>",
    lambda e: table_canvas.configure(scrollregion=table_canvas.bbox("all")),
)
table_canvas.create_window((0, 0), window=table_frame, anchor="nw")
table_canvas.configure(yscrollcommand=table_scrollbar.set)
table_canvas.pack(side="left", fill="both", expand=True)
table_scrollbar.pack(side="right", fill="y")
update_calibration_table()  # Vẽ bảng trống ban đầu

# Actions
action_frame = ttk.LabelFrame(left_column_frame, text="Actions")
action_frame.pack(fill=tk.X, pady=10)
fit_button = ttk.Button(
    action_frame,
    text="Fit Curve & Get Formula",
    command=fit_curve_and_display,
    state=tk.DISABLED,
)
fit_button.pack(side=tk.LEFT, padx=5, pady=5)
clear_button = ttk.Button(
    action_frame, text="Clear Data", command=clear_calibration_data, state=tk.DISABLED
)
clear_button.pack(side=tk.LEFT, padx=5, pady=5)

file_action_frame = ttk.LabelFrame(left_column_frame, text="File")
file_action_frame.pack(fill=tk.X, pady=5)
save_button = ttk.Button(
    file_action_frame,
    text="Save Data to CSV",
    command=save_calibration_data,
    state=tk.DISABLED,
)
save_button.pack(side=tk.LEFT, padx=5, pady=5)
load_button = ttk.Button(
    file_action_frame, text="Load Data from CSV", command=load_calibration_data
)  # Luôn enabled
load_button.pack(side=tk.LEFT, padx=5, pady=5)


# --- Cột Giữa: Đồ thị ---
plot_display_frame = ttk.LabelFrame(main_frame, text="Calibration Plot")
plot_display_frame.grid(row=0, column=1, sticky=(tk.N, tk.S, tk.E, tk.W), padx=5)
main_frame.columnconfigure(1, weight=1)  # Cho phép cột đồ thị mở rộng

fig = Figure(figsize=(6, 4), dpi=100)
ax = fig.add_subplot(111)
canvas = FigureCanvasTkAgg(fig, master=plot_display_frame)
canvas_widget = canvas.get_tk_widget()
canvas_widget.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
plot_calibration_data()  # Vẽ đồ thị trống ban đầu

# --- Cột Phải: Kết quả và Raw Serial ---
right_column_frame = ttk.Frame(main_frame, padding="5")
right_column_frame.grid(row=0, column=2, sticky=(tk.N, tk.S, tk.E), padx=5, pady=5)
main_frame.columnconfigure(2, weight=0)  # Cột này không cần mở rộng nhiều

# Result Display
result_frame = ttk.LabelFrame(right_column_frame, text="Fitting Result & C++ Code")
result_frame.pack(fill=tk.X, pady=5)
result_text = tk.StringVar()
result_text.set("Kết quả khớp và code C++ sẽ hiển thị ở đây.")
result_label = ttk.Label(
    result_frame, textvariable=result_text, wraplength=300, justify=tk.LEFT
)
result_label.pack(padx=5, pady=5, fill=tk.X)

# Raw Serial Monitor (Optional)
raw_serial_frame = ttk.LabelFrame(
    right_column_frame, text="Raw Serial Output from ESP32"
)
raw_serial_frame.pack(fill=tk.BOTH, expand=True, pady=5)
raw_serial_text = tk.Text(raw_serial_frame, height=10, width=40, wrap=tk.WORD)
raw_serial_scrollbar = ttk.Scrollbar(raw_serial_frame, command=raw_serial_text.yview)
raw_serial_text.configure(yscrollcommand=raw_serial_scrollbar.set)
raw_serial_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
raw_serial_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)


# --- Main Loop ---
root.after(100, update_gui_from_queue_calib)


def on_closing_calib():
    global serial_thread_running
    serial_thread_running = False
    time.sleep(0.1)
    if ser and ser.is_open:
        ser.close()
    root.destroy()


root.protocol("WM_DELETE_WINDOW", on_closing_calib)
root.mainloop()
