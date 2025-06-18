import sys
import time
import serial
import serial.tools.list_ports
import numpy as np

# import pandas as pd # Không thực sự cần thiết cho GUI này nếu không load/save CSV
from PyQt5.QtWidgets import (
    QApplication,
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QComboBox,
    QCheckBox,
    QTextEdit,
    QGridLayout,
    QGroupBox,
    QDoubleSpinBox,
    QMessageBox,
)
from PyQt5.QtCore import QTimer, QThread, pyqtSignal, Qt
import pyqtgraph as pg

# --- Global Configuration ---
SERIAL_BAUD_RATE = 115200
# DATA_SEND_INTERVAL_ESP_MS đã được định nghĩa trong Config.h của ESP32, Python sẽ nhận theo tần suất đó
# PYTHON_DATA_SEND_INTERVAL_MS trên ESP32 là 200ms
PLOT_UPDATE_INTERVAL_MS = 250  # Có thể nhanh hơn một chút so với tần suất gửi của ESP
MAX_PLOT_POINTS = 300


# --- Serial Communication Thread ---
class SerialThread(QThread):
    data_received = pyqtSignal(str)
    connection_status = pyqtSignal(str, str)

    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.handshake_successful = False  # Cờ cho handshake

    def run(self):
        self.running = True
        self.handshake_successful = False  # Reset cờ
        print("[Thread] SerialThread run() started.")
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            print(f"[Thread] Serial port {self.port} opened.")
            time.sleep(0.5)  # Chờ một chút cho cổng ổn định

            # --- Thực hiện Handshake ---
            print("[Thread] Sending PING to ESP32...")
            self.ser.write(b"PING\n")  # Gửi lệnh PING

            pong_received_time = time.time()
            pong_received = False
            expected_pong = "PONG_ESP32"

            while time.time() - pong_received_time < 3.0:  # Chờ PONG trong 3 giây
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode("utf-8", errors="ignore").strip()
                    print(f"[Thread] Handshake RX: '{line}'")
                    # if expected_pong in line:
                    #     self.handshake_successful = True
                    #     pong_received = True
                    #     self.connection_status.emit(
                    #         f"Connected to {self.port} (Handshake OK)", "green"
                    #     )
                    #     print(
                    #         f"[Thread] '{expected_pong}' received. Handshake successful."
                    #     )
                    #     break
                if not self.running:  # Nếu thread bị dừng từ bên ngoài
                    break
                time.sleep(0.05)  # Check thường xuyên hơn một chút

            # if not pong_received and self.running:
            #     self.connection_status.emit(
            #         f"Handshake failed on {self.port}. No '{expected_pong}'.", "red"
            #     )
            #     print(f"[Thread] Handshake FAILED. No '{expected_pong}' response.")
            #     if self.ser:
            #         self.ser.close()
            #     self.ser = None
            #     self.running = False
            #     return

            # if not self.running:  # Nếu thread bị dừng trong lúc handshake
            #     if self.ser:
            #         self.ser.close()
            #     self.ser = None
            #     return

            while self.running:
                if self.ser and self.ser.in_waiting > 0:
                    try:
                        line = (
                            self.ser.readline().decode("utf-8", errors="ignore").strip()
                        )
                        if line:
                            self.data_received.emit(line)
                    except serial.SerialException as e:
                        self.connection_status.emit(f"Serial error: {e}", "red")
                        self.running = False
                        break
                    except Exception as e_read:
                        print(f"Error reading line: {e_read}")
                time.sleep(0.005)

        except serial.SerialException as e:
            print(f"[Thread] SerialException in run() during connection/handshake: {e}")
            self.connection_status.emit(f"Connection/Handshake failed: {e}", "red")
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()
            # Chỉ cập nhật Disconnected nếu không phải do lỗi đã báo trước
            # hoặc nếu handshake không thành công và chưa báo Disconnected
            if self.running is False and not self.handshake_successful:
                # Nếu running là False do stop() thì không cần báo lại Disconnected ở đây
                # Chỉ báo nếu nó dừng đột ngột hoặc handshake fail
                if (
                    "failed" not in self.parent().status_label.text().lower()
                ):  # Tránh lặp lại thông báo lỗi
                    self.connection_status.emit("Disconnected (Thread Ended)", "black")

            self.ser = None
            print("[Thread] SerialThread run() finished.")

    def send_command(self, command):
        if self.ser and self.ser.is_open and self.running:
            try:
                self.ser.write((command + "\n").encode("utf-8"))
                print(f"[Thread] Sent: {command}")  # DEBUG
                return True
            except Exception as e:
                self.connection_status.emit(f"Error sending: {e}", "red")
                return False
        print("[Thread] Cannot send, serial not open or not running.")  # DEBUG
        return False

    def stop(self):
        print("[Thread] stop() called.")  # DEBUG
        self.running = False
        # Không cần self.wait() ở đây nếu QMainWindow.closeEvent() xử lý đúng cách
        # Hoặc nếu cần, đảm bảo nó không block GUI quá lâu.
        # Một cách khác là chờ trong closeEvent của Main Window.


# --- Main Application Window ---
class PIDTunerWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        # ... (Phần initUI giữ nguyên như code tool "xịn" trước,
        #      bao gồm các spinbox, button, label, và setup đồ thị pyqtgraph) ...
        self.setWindowTitle("ESP32 PID Tuner (Schematic Mode)")
        self.setGeometry(100, 100, 1000, 750)  # Tăng chiều cao một chút cho Z-N

        self.serial_thread = None
        self.plot_data = {  # Các key này phải khớp với những gì bạn muốn vẽ
            "dist_r": pg.mkPen(color="c", width=2),
            "dist_f": pg.mkPen(color="m", width=2),
            "target_r": pg.mkPen(color="g", width=1, style=Qt.DashLine),
            "error_r": pg.mkPen(color="y", width=1),
            "steer": pg.mkPen(color="r", width=2),
        }
        self.data_arrays = {key: np.zeros(MAX_PLOT_POINTS) for key in self.plot_data}
        # Tạo trục thời gian giả định, sẽ tốt hơn nếu ESP32 gửi timestamp
        self.time_axis_data = np.arange(MAX_PLOT_POINTS) * (
            200 / 1000.0
        )  # Giả sử PYTHON_DATA_SEND_INTERVAL_MS là 200ms
        self.ptr = 0

        self.initUI()  # Gọi hàm setup GUI
        self.update_ports_list()

    def initUI(self):  # Rút gọn, các phần tử GUI giống như code "xịn" trước
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout(central_widget)

        controls_panel = QWidget()
        controls_layout = QVBoxLayout(controls_panel)
        main_layout.addWidget(controls_panel, 1)

        # Connection Group
        conn_group = QGroupBox("Serial Connection")
        conn_layout = QGridLayout()
        conn_group.setLayout(conn_layout)
        self.port_combo = QComboBox()
        self.refresh_ports_button = QPushButton("Refresh Ports")
        self.refresh_ports_button.clicked.connect(self.update_ports_list)
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.toggle_connection)
        self.status_label = QLabel("Disconnected")
        conn_layout.addWidget(QLabel("Port:"), 0, 0)
        conn_layout.addWidget(self.port_combo, 0, 1)
        conn_layout.addWidget(self.refresh_ports_button, 0, 2)
        conn_layout.addWidget(self.connect_button, 1, 0, 1, 3)
        conn_layout.addWidget(self.status_label, 2, 0, 1, 3)
        controls_layout.addWidget(conn_group)

        # PID Parameters Group
        pid_params_group = QGroupBox("PID Parameters (Wall Following)")
        pid_params_layout = QGridLayout()
        pid_params_group.setLayout(pid_params_layout)
        self.kp_spin = QDoubleSpinBox()
        self.kp_spin.setRange(0, 100)
        self.kp_spin.setSingleStep(0.01)
        self.kp_spin.setValue(0.8)
        self.kp_spin.setDecimals(3)
        self.ki_spin = QDoubleSpinBox()
        self.ki_spin.setRange(0, 10)
        self.ki_spin.setSingleStep(0.001)
        self.ki_spin.setValue(0.02)
        self.ki_spin.setDecimals(4)
        self.kd_spin = QDoubleSpinBox()
        self.kd_spin.setRange(0, 10)
        self.kd_spin.setSingleStep(0.01)
        self.kd_spin.setValue(0.1)
        self.kd_spin.setDecimals(3)
        self.target_dist_spin = QDoubleSpinBox()
        self.target_dist_spin.setRange(10, 100)
        self.target_dist_spin.setSingleStep(1.0)
        self.target_dist_spin.setValue(25.0)  # Khớp với ESP32
        self.update_pid_button = QPushButton("Send PID & Target")
        self.update_pid_button.clicked.connect(self.send_pid_and_target_params)
        pid_params_layout.addWidget(QLabel("Kp:"), 0, 0)
        pid_params_layout.addWidget(self.kp_spin, 0, 1)
        pid_params_layout.addWidget(QLabel("Ki:"), 1, 0)
        pid_params_layout.addWidget(self.ki_spin, 1, 1)
        pid_params_layout.addWidget(QLabel("Kd:"), 2, 0)
        pid_params_layout.addWidget(self.kd_spin, 2, 1)
        pid_params_layout.addWidget(QLabel("Target Dist (cm):"), 3, 0)
        pid_params_layout.addWidget(self.target_dist_spin, 3, 1)
        pid_params_layout.addWidget(self.update_pid_button, 4, 0, 1, 2)
        controls_layout.addWidget(pid_params_group)

        # Robot Control Group
        robot_control_group = QGroupBox("Robot Control")
        robot_control_layout = QHBoxLayout()
        robot_control_group.setLayout(robot_control_layout)
        self.start_button = QPushButton("START Robot")  # Khớp với lệnh ESP32
        self.start_button.clicked.connect(lambda: self.send_robot_command_gui("START"))
        self.stop_button = QPushButton("STOP Robot")  # Khớp với lệnh ESP32
        self.stop_button.clicked.connect(
            lambda: self.send_robot_command_gui("STOP_ROBOT")
        )
        robot_control_layout.addWidget(self.start_button)
        robot_control_layout.addWidget(self.stop_button)
        controls_layout.addWidget(robot_control_group)

        # Ziegler-Nichols Helper
        zn_group = QGroupBox("Ziegler-Nichols Helper (Ultimate Sensitivity)")
        zn_layout = QGridLayout()
        zn_group.setLayout(zn_layout)
        self.ku_input = QDoubleSpinBox()
        self.ku_input.setRange(0, 100)
        self.ku_input.setSingleStep(0.1)
        self.tu_input = QDoubleSpinBox()
        self.tu_input.setRange(0.1, 60)
        self.tu_input.setSingleStep(0.1)
        self.calculate_zn_button = QPushButton("Calculate Z-N PID")
        self.calculate_zn_button.clicked.connect(self.calculate_ziegler_nichols)
        self.zn_kp_label = QLabel("Suggested Kp: N/A")
        self.zn_ki_label = QLabel("Suggested Ki: N/A")
        self.zn_kd_label = QLabel("Suggested Kd: N/A")
        zn_layout.addWidget(QLabel("Enter Ku:"), 0, 0)
        zn_layout.addWidget(self.ku_input, 0, 1)
        zn_layout.addWidget(QLabel("Enter Tu (s):"), 1, 0)
        zn_layout.addWidget(self.tu_input, 1, 1)
        zn_layout.addWidget(self.calculate_zn_button, 2, 0, 1, 2)
        zn_layout.addWidget(self.zn_kp_label, 3, 0, 1, 2)
        zn_layout.addWidget(self.zn_ki_label, 4, 0, 1, 2)
        zn_layout.addWidget(self.zn_kd_label, 5, 0, 1, 2)
        controls_layout.addWidget(zn_group)

        # Live Data Display
        live_data_group = QGroupBox("Live Data from ESP32")
        live_data_layout = QGridLayout()
        live_data_group.setLayout(live_data_layout)
        self.state_val_label = QLabel("IDLE")
        self.dist_r_val_label = QLabel("N/A cm")
        self.dist_f_val_label = QLabel("N/A cm")
        self.target_r_val_label = QLabel("N/A cm")
        self.error_r_val_label = QLabel("N/A cm")
        self.steer_val_label = QLabel("N/A")
        self.pwml_val_label = QLabel("N/A")
        self.pwmr_val_label = QLabel("N/A")
        self.esp_kp_val_label = QLabel("N/A")
        self.esp_ki_val_label = QLabel("N/A")
        self.esp_kd_val_label = QLabel("N/A")
        self.py_ctrl_label = QLabel("ESP32")  # Ban đầu ESP32 tự kiểm soát Kx

        live_data_layout.addWidget(QLabel("State:"), 0, 0)
        live_data_layout.addWidget(self.state_val_label, 0, 1)
        live_data_layout.addWidget(QLabel("Dist R:"), 1, 0)
        live_data_layout.addWidget(self.dist_r_val_label, 1, 1)
        live_data_layout.addWidget(QLabel("Dist F:"), 1, 2)
        live_data_layout.addWidget(self.dist_f_val_label, 1, 3)
        live_data_layout.addWidget(QLabel("Target R:"), 2, 0)
        live_data_layout.addWidget(self.target_r_val_label, 2, 1)
        live_data_layout.addWidget(QLabel("Error R:"), 2, 2)
        live_data_layout.addWidget(self.error_r_val_label, 2, 3)
        live_data_layout.addWidget(QLabel("Steer Adj:"), 3, 0)
        live_data_layout.addWidget(self.steer_val_label, 3, 1)
        live_data_layout.addWidget(QLabel("PWM L:"), 4, 0)
        live_data_layout.addWidget(self.pwml_val_label, 4, 1)
        live_data_layout.addWidget(QLabel("PWM R:"), 4, 2)
        live_data_layout.addWidget(self.pwmr_val_label, 4, 3)
        live_data_layout.addWidget(QLabel("ESP Kp:"), 5, 0)
        live_data_layout.addWidget(self.esp_kp_val_label, 5, 1)
        live_data_layout.addWidget(QLabel("ESP Ki:"), 6, 0)
        live_data_layout.addWidget(self.esp_ki_val_label, 6, 1)
        live_data_layout.addWidget(QLabel("ESP Kd:"), 7, 0)
        live_data_layout.addWidget(self.esp_kd_val_label, 7, 1)
        live_data_layout.addWidget(QLabel("Kx Ctrl by:"), 5, 2)
        live_data_layout.addWidget(self.py_ctrl_label, 5, 3)  # Ai đang control Kx
        controls_layout.addWidget(live_data_group)
        controls_layout.addStretch(1)

        # Right Panel: Plot
        plot_widget = pg.GraphicsLayoutWidget()
        main_layout.addWidget(plot_widget, 3)

        self.p_dist = plot_widget.addPlot(row=0, col=0, title="Distances & Target")
        self.p_dist.addLegend(offset=(-10, 10))  # Điều chỉnh vị trí legend
        self.curve_dist_r = self.p_dist.plot(
            pen=self.plot_data["dist_r"], name="Dist R"
        )
        self.curve_dist_f = self.p_dist.plot(
            pen=self.plot_data["dist_f"], name="Dist F"
        )
        self.curve_target_r = self.p_dist.plot(
            pen=self.plot_data["target_r"], name="Target R"
        )
        self.p_dist.setLabel("left", "Distance (cm)")
        self.p_dist.showGrid(x=True, y=True)

        plot_widget.nextRow()
        self.p_steer = plot_widget.addPlot(
            row=1, col=0, title="Error & Steering Adjustment"
        )
        self.p_steer.addLegend(offset=(-10, 10))
        self.curve_error_r = self.p_steer.plot(
            pen=self.plot_data["error_r"], name="Error R"
        )
        self.curve_steer = self.p_steer.plot(
            pen=self.plot_data["steer"], name="Steer Adj."
        )
        self.p_steer.setLabel("left", "Value")
        self.p_steer.setLabel("bottom", "Time Index")
        self.p_steer.showGrid(x=True, y=True)

        self.plot_timer = QTimer()
        self.plot_timer.timeout.connect(self.update_plot)
        # plot_timer sẽ được start sau khi kết nối thành công

    # ... (giữ nguyên các hàm update_ports_list, toggle_connection, send_pid_and_target_params, send_robot_command_gui, calculate_ziegler_nichols) ...

    def update_connection_status(self, message, color):
        self.status_label.setText(message)
        self.status_label.setStyleSheet(f"color: {color};")
        if "Connected" in message:
            self.connect_button.setText("Disconnect")
            self.plot_timer.start(
                PLOT_UPDATE_INTERVAL_MS
            )  # Bắt đầu vẽ đồ thị khi kết nối
            print("[GUI] Plot timer started.")
        else:
            self.connect_button.setText("Connect")
            self.plot_timer.stop()  # Dừng đồ thị khi mất kết nối
            print("[GUI] Plot timer stopped.")

    def handle_serial_data(self, line):
        # print(f"GUI RX: {line}") # For debugging
        if line.startswith("DATA,"):
            parts = line.split(",")
            # DATA,state,distR,distF,targetR,errorR,steer,pwmL,pwmR,kp,ki,kd,pythonPidControlEnabled
            if len(parts) == 13:
                try:
                    state_code = int(parts[1])
                    states = {
                        0: "IDLE",
                        1: "WALL_FOL",
                        2: "OBST_AHD",
                        3: "TURN_L",
                        4: "TURN_R",
                    }
                    self.state_val_label.setText(states.get(state_code, "UNKNOWN"))

                    dist_r = float(parts[2])
                    self.dist_r_val_label.setText(f"{dist_r:.1f} cm")
                    dist_f = float(parts[3])
                    self.dist_f_val_label.setText(f"{dist_f:.1f} cm")
                    target_r = float(parts[4])
                    self.target_r_val_label.setText(f"{target_r:.1f} cm")
                    error_r = float(parts[5])
                    self.error_r_val_label.setText(f"{error_r:.1f} cm")
                    steer = float(parts[6])
                    self.steer_val_label.setText(f"{steer:.2f}")
                    self.pwml_val_label.setText(parts[7])
                    self.pwmr_val_label.setText(parts[8])

                    # Cập nhật giá trị Kx từ ESP32 vào spinbox nếu Python không phải là người gửi lệnh Kx gần nhất
                    # Hoặc đơn giản là luôn cập nhật để đồng bộ
                    self.esp_kp_val_label.setText(f"{float(parts[9]):.3f}")
                    self.esp_ki_val_label.setText(f"{float(parts[10]):.4f}")
                    self.esp_kd_val_label.setText(f"{float(parts[11]):.3f}")

                    # Nếu người dùng không đang nhập liệu vào các ô Kx, cập nhật chúng
                    if self.focusWidget() not in [
                        self.kp_spin,
                        self.ki_spin,
                        self.kd_spin,
                    ]:
                        self.kp_spin.setValue(float(parts[9]))
                        self.ki_spin.setValue(float(parts[10]))
                        self.kd_spin.setValue(float(parts[11]))

                    self.py_ctrl_label.setText(
                        "PYTHON" if int(parts[12]) == 1 else "ESP32"
                    )

                    # Update plot data arrays (circular buffer)
                    self.data_arrays["dist_r"][self.ptr] = dist_r
                    self.data_arrays["dist_f"][self.ptr] = dist_f
                    self.data_arrays["target_r"][self.ptr] = target_r
                    self.data_arrays["error_r"][self.ptr] = error_r
                    self.data_arrays["steer"][self.ptr] = steer

                    self.ptr = (self.ptr + 1) % MAX_PLOT_POINTS

                except ValueError as e:
                    print(f"ValueError parsing GUI DATA: {e} - Line: {line}")
                except Exception as e_parse:
                    print(f"Error parsing GUI DATA: {e_parse} - Line: {line}")

        elif line.startswith("ACK_"):
            ack_msg_parts = line.split(":")
            ack_type = ack_msg_parts[0]
            ack_val = ack_msg_parts[1] if len(ack_msg_parts) > 1 else ""
            self.status_label.setText(
                f"ESP Ack: {ack_type}{': '+ack_val if ack_val else ''}"
            )
            self.status_label.setStyleSheet("color: blue;")
            # Nếu là ACK Kx, cập nhật lại giá trị Kx trên ESP label
            if ack_type == "ACK_KP":
                self.esp_kp_val_label.setText(ack_val)
            elif ack_type == "ACK_KI":
                self.esp_ki_val_label.setText(ack_val)
            elif ack_type == "ACK_KD":
                self.esp_kd_val_label.setText(ack_val)
            elif ack_type == "ACK_TARGET":
                self.target_r_val_label.setText(f"{float(ack_val):.1f} cm")

        elif (
            line.strip() and not "ESP_READY" in line
        ):  # Bỏ qua các dòng ESP_READY... ở đây
            print(f"ESP32 Info: {line}")

    def update_plot(self):  # Đổi tên từ update_plot_manual_time
        # Tạo ra một view "cuộn" của dữ liệu bằng cách dịch chuyển con trỏ
        # Dữ liệu mới nhất sẽ ở self.ptr - 1 (nếu self.ptr chưa quay vòng)
        # Hoặc nếu self.ptr đã quay vòng, dữ liệu sẽ được sắp xếp lại.

        # Sắp xếp lại dữ liệu để vẽ đúng thứ tự thời gian
        # Phần tử tại self.ptr là phần tử cũ nhất trong buffer tròn
        rolled_dist_r = np.roll(self.data_arrays["dist_r"], -self.ptr)
        rolled_dist_f = np.roll(self.data_arrays["dist_f"], -self.ptr)
        rolled_target_r = np.roll(self.data_arrays["target_r"], -self.ptr)
        rolled_error_r = np.roll(self.data_arrays["error_r"], -self.ptr)
        rolled_steer = np.roll(self.data_arrays["steer"], -self.ptr)

        self.curve_dist_r.setData(self.time_axis_data, rolled_dist_r)
        self.curve_dist_f.setData(self.time_axis_data, rolled_dist_f)
        self.curve_target_r.setData(self.time_axis_data, rolled_target_r)

        self.curve_error_r.setData(self.time_axis_data, rolled_error_r)
        self.curve_steer.setData(self.time_axis_data, rolled_steer)

    def closeEvent(self, event):
        print("[GUI] closeEvent called.")  # DEBUG
        if self.serial_thread and self.serial_thread.isRunning():
            print("[GUI] Stopping serial thread from closeEvent...")  # DEBUG
            self.serial_thread.stop()
            self.serial_thread.wait(2000)  # Chờ thread dừng, tối đa 2 giây
            print("[GUI] Serial thread should be stopped.")  # DEBUG
        self.plot_timer.stop()  # Dừng timer đồ thị
        print("[GUI] Plot timer stopped from closeEvent.")  # DEBUG
        event.accept()

    def update_ports_list(self):
        self.port_combo.clear()
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo.addItems(ports)
        if ports:
            self.port_combo.setCurrentIndex(0)

    def toggle_connection(self):
        print("[GUI] toggle_connection called.")
        if self.serial_thread and self.serial_thread.isRunning():
            print("[GUI] Attempting to stop serial thread.")
            self.serial_thread.stop()
            self.serial_thread.wait(1000)
            # Trạng thái sẽ được cập nhật bởi on_serial_thread_finished hoặc connection_status từ thread
            # self.update_connection_status("Disconnecting...", "orange") # Có thể thêm để phản hồi ngay
        else:
            port = self.port_combo.currentText()
            print(f"[GUI] Attempting to connect to port: {port}")
            if not port or "No ports found" in port or "Error listing ports" in port:
                QMessageBox.warning(
                    self, "Port Error", "No valid serial port selected or available."
                )
                return

            # Reset dữ liệu đồ thị
            for key in self.data_arrays:
                if key != "time_internal":
                    self.data_arrays[key].fill(0)
            self.ptr = 0
            print("[GUI] Plot data arrays reset.")

            self.status_label.setText(
                f"Connecting to {port}..."
            )  # Thông báo đang kết nối
            self.status_label.setStyleSheet("color: orange;")

            self.serial_thread = SerialThread(port, SERIAL_BAUD_RATE)
            self.serial_thread.setParent(
                self
            )  # Để quản lý vòng đời thread tốt hơn với QObject
            self.serial_thread.data_received.connect(self.handle_serial_data)
            self.serial_thread.connection_status.connect(self.update_connection_status)
            self.serial_thread.finished.connect(self.on_serial_thread_finished)
            self.serial_thread.start()
            print("[GUI] SerialThread start() called.")

    def on_serial_thread_finished(self):
        print("[GUI] Serial thread finished signal received.")
        # Chỉ cập nhật GUI nếu kết nối không được thiết lập thành công
        # hoặc nếu thread dừng mà không phải do người dùng nhấn Disconnect
        if not (
            self.serial_thread
            and self.serial_thread.handshake_successful
            and "Connected" in self.status_label.text()
        ):
            self.update_connection_status("Disconnected (Thread Ended)", "black")
        self.serial_thread = None  # Dọn dẹp

    def update_connection_status(self, message, color):
        print(f"[GUI] update_connection_status: '{message}', color: {color}")
        self.status_label.setText(message)
        self.status_label.setStyleSheet(f"color: {color};")
        if "Connected" in message:
            self.connect_button.setText("Disconnect")
            if not self.plot_timer.isActive():
                self.plot_timer.start(PLOT_UPDATE_INTERVAL_MS)
                print("[GUI] Plot timer started by update_connection_status.")
        else:  # Disconnected hoặc Error
            self.connect_button.setText("Connect")
            if self.plot_timer.isActive():
                self.plot_timer.stop()
                print("[GUI] Plot timer stopped by update_connection_status.")
            # Nếu thread đã dừng và không phải đang trong quá trình ngắt kết nối chủ động
            # thì gán self.serial_thread = None
            if (
                self.serial_thread
                and not self.serial_thread.isRunning()
                and "Disconnecting" not in message
            ):
                self.serial_thread = None

    def send_pid_and_target_params(self):
        if self.serial_thread and self.serial_thread.isRunning():
            kp = self.kp_spin.value()
            ki = self.ki_spin.value()
            kd = self.kd_spin.value()
            target = self.target_dist_spin.value()

            # Send commands sequentially with small delay
            self.serial_thread.send_command(f"KP:{kp:.3f}")
            QThread.msleep(50)  # PyQt5 alternative to time.sleep in GUI thread
            self.serial_thread.send_command(f"KI:{ki:.4f}")
            QThread.msleep(50)
            self.serial_thread.send_command(f"KD:{kd:.3f}")
            QThread.msleep(50)
            self.serial_thread.send_command(f"TARGET:{target:.1f}")
        else:
            QMessageBox.warning(self, "Serial Error", "Not connected to ESP32.")

    def send_robot_command_gui(self, command):
        if self.serial_thread and self.serial_thread.isRunning():
            self.serial_thread.send_command(command)
        else:
            QMessageBox.warning(self, "Serial Error", "Not connected to ESP32.")

    def calculate_ziegler_nichols(self):
        try:
            Ku = self.ku_input.value()
            Tu = self.tu_input.value()  # In seconds
            if Ku <= 0 or Tu <= 0:
                QMessageBox.warning(self, "Input Error", "Ku and Tu must be positive.")
                return

            # Classic Z-N rules for PID
            zn_kp = 0.6 * Ku
            zn_ti = Tu / 2.0
            zn_td = Tu / 8.0
            zn_ki = zn_kp / zn_ti if zn_ti > 0 else 0
            zn_kd = zn_kp * zn_td

            self.zn_kp_label.setText(f"Suggested Kp (Classic): {zn_kp:.3f}")
            self.zn_ki_label.setText(f"Suggested Ki (Classic): {zn_ki:.4f}")
            self.zn_kd_label.setText(f"Suggested Kd (Classic): {zn_kd:.3f}")

            # Optionally update the spin boxes
            self.kp_spin.setValue(zn_kp)
            self.ki_spin.setValue(zn_ki)
            self.kd_spin.setValue(zn_kd)

        except Exception as e:
            QMessageBox.critical(self, "Calculation Error", f"Error: {e}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    pg.setConfigOption("background", "w")
    pg.setConfigOption("foreground", "k")
    mainWin = PIDTunerWindow()
    mainWin.show()
    sys.exit(app.exec_())
