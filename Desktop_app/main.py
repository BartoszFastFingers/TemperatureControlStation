import sys
import time
import struct
from datetime import datetime
from collections import deque

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QLineEdit, QComboBox, QGroupBox, QGridLayout,
    QStatusBar, QMessageBox, QSpinBox, QDoubleSpinBox, QFrame
)
from PyQt6.QtCore import QTimer, Qt, QThread, pyqtSignal
from PyQt6.QtGui import QFont, QColor, QPalette

import serial
import serial.tools.list_ports

import pyqtgraph as pg
import numpy as np


# ============ CRC8 ============

CRC8_TABLE = [
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
]

def crc8(data: bytes) -> int:
    """Oblicza CRC8 dla danych"""
    crc = 0x00
    for byte in data:
        crc = CRC8_TABLE[crc ^ byte]
    return crc

def make_command(cmd: str) -> bytes:
    """Tworzy komendę z CRC"""
    crc = crc8(cmd.encode())
    return f"{cmd}*{crc:02X}\r\n".encode()

def verify_message(msg: str) -> tuple:
    """Weryfikuje wiadomość i zwraca (valid, data)"""
    try:
        if '*' not in msg:
            return False, None
        
        data_part, crc_part = msg.rsplit('*', 1)
        crc_received = int(crc_part.strip(), 16)
        crc_calculated = crc8(data_part.encode())
        
        if crc_received != crc_calculated:
            return False, None
        
        return True, data_part
    except:
        return False, None


# ============ SERIAL THREAD ============

class SerialThread(QThread):
    """Wątek do obsługi komunikacji szeregowej"""
    
    data_received = pyqtSignal(float, float, int, int, int)  # temp, setpoint, heater, fan, timestamp
    connection_status = pyqtSignal(bool, str)
    ack_received = pyqtSignal(str)
    error_received = pyqtSignal(str)
    
    def __init__(self):
        super().__init__()
        self.serial_port = None
        self.running = False
        self.port_name = ""
        self.baudrate = 115200
        
    def connect(self, port: str, baudrate: int = 115200):
        """Połącz z portem szeregowym"""
        try:
            self.serial_port = serial.Serial(port, baudrate, timeout=0.1)
            self.port_name = port
            self.baudrate = baudrate
            self.running = True
            self.connection_status.emit(True, f"Połączono z {port}")
            self.start()
        except Exception as e:
            self.connection_status.emit(False, f"Błąd: {str(e)}")
    
    def disconnect(self):
        """Rozłącz"""
        self.running = False
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        self.connection_status.emit(False, "Rozłączono")
    
    def send_command(self, cmd: str):
        """Wyślij komendę"""
        if self.serial_port and self.serial_port.is_open:
            command = make_command(cmd)
            self.serial_port.write(command)
    
    def run(self):
        """Główna pętla wątku"""
        buffer = ""
        
        while self.running:
            try:
                if self.serial_port and self.serial_port.is_open:
                    if self.serial_port.in_waiting > 0:
                        data = self.serial_port.read(self.serial_port.in_waiting)
                        buffer += data.decode('utf-8', errors='ignore')
                        
                        # Przetwarzaj kompletne linie
                        while '\n' in buffer:
                            line, buffer = buffer.split('\n', 1)
                            line = line.strip()
                            
                            if line.startswith('$'):
                                self.process_message(line)
                
                self.msleep(10)  # 10ms delay
                
            except Exception as e:
                self.error_received.emit(str(e))
                self.msleep(100)
    
    def process_message(self, msg: str):
        """Przetwarzanie odebranej wiadomości"""
        valid, data = verify_message(msg)
        
        if not valid:
            self.error_received.emit(f"CRC Error: {msg}")
            return
        
        try:
            if data.startswith('$DATA,'):
                # $DATA,temp,setpoint,heater_pwm,fan_pwm,timestamp
                parts = data[6:].split(',')
                if len(parts) >= 5:
                    temp = float(parts[0])
                    setpoint = float(parts[1])
                    heater = int(parts[2])
                    fan = int(parts[3])
                    timestamp = int(parts[4])
                    self.data_received.emit(temp, setpoint, heater, fan, timestamp)
            
            elif data.startswith('$ACK,'):
                self.ack_received.emit(data[5:])
            
            elif data.startswith('$NAK,'):
                self.error_received.emit(f"NAK: {data[5:]}")
            
            elif data.startswith('$PONG'):
                self.ack_received.emit("PONG")
                
        except Exception as e:
            self.error_received.emit(f"Parse error: {str(e)}")


# ============ MAIN WINDOW ============

class TemperatureMonitor(QMainWindow):
    """Główne okno aplikacji"""
    
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle("Temperature Control Monitor")
        self.setMinimumSize(900, 700)
        
        # Dane do wykresu
        self.max_points = 300  # 5 minut przy 1s próbkowania
        self.time_data = deque(maxlen=self.max_points)
        self.temp_data = deque(maxlen=self.max_points)
        self.setpoint_data = deque(maxlen=self.max_points)
        self.heater_data = deque(maxlen=self.max_points)
        self.fan_data = deque(maxlen=self.max_points)
        
        # Serial thread
        self.serial_thread = SerialThread()
        self.serial_thread.data_received.connect(self.on_data_received)
        self.serial_thread.connection_status.connect(self.on_connection_status)
        self.serial_thread.ack_received.connect(self.on_ack_received)
        self.serial_thread.error_received.connect(self.on_error_received)
        
        # UI
        self.init_ui()
        
        # Timer do odświeżania portów
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.refresh_ports)
        self.refresh_timer.start(2000)
        
        self.refresh_ports()
    
    def init_ui(self):
        """Inicjalizacja interfejsu użytkownika"""
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout(central_widget)
        
        # ===== Górny panel - połączenie =====
        conn_group = QGroupBox("Połączenie")
        conn_layout = QHBoxLayout(conn_group)
        
        conn_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(150)
        conn_layout.addWidget(self.port_combo)
        
        conn_layout.addWidget(QLabel("Baudrate:"))
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.setCurrentText("115200")
        conn_layout.addWidget(self.baud_combo)
        
        self.connect_btn = QPushButton("Połącz")
        self.connect_btn.clicked.connect(self.toggle_connection)
        conn_layout.addWidget(self.connect_btn)
        
        self.ping_btn = QPushButton("Ping")
        self.ping_btn.clicked.connect(self.send_ping)
        self.ping_btn.setEnabled(False)
        conn_layout.addWidget(self.ping_btn)
        
        conn_layout.addStretch()
        main_layout.addWidget(conn_group)
        
        middle_layout = QHBoxLayout()
        
        temp_group = QGroupBox("Temperatura")
        temp_layout = QGridLayout(temp_group)
        
        self.temp_label = QLabel("--.-")
        self.temp_label.setFont(QFont("Arial", 48, QFont.Weight.Bold))
        self.temp_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.temp_label.setStyleSheet("color: #00AA00;")
        temp_layout.addWidget(self.temp_label, 0, 0, 1, 2)
        
        temp_layout.addWidget(QLabel("°C"), 0, 2)
        
        temp_layout.addWidget(QLabel("Setpoint:"), 1, 0)
        self.setpoint_display = QLabel("--.- °C")
        self.setpoint_display.setFont(QFont("Arial", 14))
        temp_layout.addWidget(self.setpoint_display, 1, 1)
        
        # Error
        temp_layout.addWidget(QLabel("Błąd:"), 2, 0)
        self.error_display = QLabel("--.- °C")
        self.error_display.setFont(QFont("Arial", 14))
        temp_layout.addWidget(self.error_display, 2, 1)
        
        middle_layout.addWidget(temp_group)
        
        # --- Sterowanie ---
        control_group = QGroupBox("Sterowanie")
        control_layout = QGridLayout(control_group)
        
        control_layout.addWidget(QLabel("Nowy setpoint:"), 0, 0)
        self.setpoint_spin = QDoubleSpinBox()
        self.setpoint_spin.setRange(20.0, 80.0)
        self.setpoint_spin.setValue(35.0)
        self.setpoint_spin.setSuffix(" °C")
        self.setpoint_spin.setDecimals(1)
        control_layout.addWidget(self.setpoint_spin, 0, 1)
        
        self.set_temp_btn = QPushButton("Ustaw")
        self.set_temp_btn.clicked.connect(self.set_temperature)
        self.set_temp_btn.setEnabled(False)
        control_layout.addWidget(self.set_temp_btn, 0, 2)
        
        self.start_btn = QPushButton("START")
        self.start_btn.clicked.connect(self.send_start)
        self.start_btn.setEnabled(False)
        self.start_btn.setStyleSheet("background-color: #00AA00; color: white;")
        control_layout.addWidget(self.start_btn, 1, 0)
        
        self.stop_btn = QPushButton("STOP")
        self.stop_btn.clicked.connect(self.send_stop)
        self.stop_btn.setEnabled(False)
        self.stop_btn.setStyleSheet("background-color: #AA0000; color: white;")
        control_layout.addWidget(self.stop_btn, 1, 1)
        
        middle_layout.addWidget(control_group)
        
        pwm_group = QGroupBox("Status PWM")
        pwm_layout = QGridLayout(pwm_group)
        
        pwm_layout.addWidget(QLabel("Grzałka:"), 0, 0)
        self.heater_bar = pg.QtWidgets.QProgressBar()
        self.heater_bar.setRange(0, 999)
        self.heater_bar.setFormat("%v / 999")
        self.heater_bar.setStyleSheet("QProgressBar::chunk { background-color: #FF4444; }")
        pwm_layout.addWidget(self.heater_bar, 0, 1)
        
        pwm_layout.addWidget(QLabel("Wentylator:"), 1, 0)
        self.fan_bar = pg.QtWidgets.QProgressBar()
        self.fan_bar.setRange(0, 999)
        self.fan_bar.setFormat("%v / 999")
        self.fan_bar.setStyleSheet("QProgressBar::chunk { background-color: #4444FF; }")
        pwm_layout.addWidget(self.fan_bar, 1, 1)
        
        middle_layout.addWidget(pwm_group)
        
        main_layout.addLayout(middle_layout)
        
        # ===== Wykres =====
        graph_group = QGroupBox("Wykres temperatury")
        graph_layout = QVBoxLayout(graph_group)
        
        # PyQtGraph widget
        self.graph_widget = pg.PlotWidget()
        self.graph_widget.setBackground('w')
        self.graph_widget.setLabel('left', 'Temperatura', units='°C')
        self.graph_widget.setLabel('bottom', 'Czas', units='s')
        self.graph_widget.addLegend()
        self.graph_widget.showGrid(x=True, y=True)
        
        # Krzywe
        self.temp_curve = self.graph_widget.plot(pen=pg.mkPen('g', width=2), name='Temperatura')
        self.setpoint_curve = self.graph_widget.plot(pen=pg.mkPen('r', width=2, style=Qt.PenStyle.DashLine), name='Setpoint')
        
        graph_layout.addWidget(self.graph_widget)
        
        # Drugi wykres - PWM
        self.pwm_graph = pg.PlotWidget()
        self.pwm_graph.setBackground('w')
        self.pwm_graph.setLabel('left', 'PWM', units='')
        self.pwm_graph.setLabel('bottom', 'Czas', units='s')
        self.pwm_graph.addLegend()
        self.pwm_graph.showGrid(x=True, y=True)
        self.pwm_graph.setYRange(0, 1000)
        
        self.heater_curve = self.pwm_graph.plot(pen=pg.mkPen('r', width=2), name='Grzałka')
        self.fan_curve = self.pwm_graph.plot(pen=pg.mkPen('b', width=2), name='Wentylator')
        
        graph_layout.addWidget(self.pwm_graph)
        
        main_layout.addWidget(graph_group)
        
        # ===== Status bar =====
        self.statusBar = QStatusBar()
        self.setStatusBar(self.statusBar)
        self.statusBar.showMessage("Rozłączono")
    
    def refresh_ports(self):
        """Odśwież listę portów"""
        current = self.port_combo.currentText()
        self.port_combo.clear()
        
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}")
        
        # Przywróć poprzedni wybór
        for i in range(self.port_combo.count()):
            if current in self.port_combo.itemText(i):
                self.port_combo.setCurrentIndex(i)
                break
    
    def toggle_connection(self):
        """Przełącz połączenie"""
        if self.serial_thread.running:
            self.serial_thread.disconnect()
        else:
            port_text = self.port_combo.currentText()
            if port_text:
                port = port_text.split(' - ')[0]
                baudrate = int(self.baud_combo.currentText())
                self.serial_thread.connect(port, baudrate)
    
    def on_connection_status(self, connected: bool, message: str):
        """Obsługa zmiany statusu połączenia"""
        self.statusBar.showMessage(message)
        
        if connected:
            self.connect_btn.setText("Rozłącz")
            self.connect_btn.setStyleSheet("background-color: #AA0000; color: white;")
            self.ping_btn.setEnabled(True)
            self.set_temp_btn.setEnabled(True)
            self.start_btn.setEnabled(True)
            self.stop_btn.setEnabled(True)
        else:
            self.connect_btn.setText("Połącz")
            self.connect_btn.setStyleSheet("")
            self.ping_btn.setEnabled(False)
            self.set_temp_btn.setEnabled(False)
            self.start_btn.setEnabled(False)
            self.stop_btn.setEnabled(False)
    
    def on_data_received(self, temp: float, setpoint: float, heater: int, fan: int, timestamp: int):
        """Obsługa odebranych danych"""
        self.temp_label.setText(f"{temp:.1f}")
        self.setpoint_display.setText(f"{setpoint:.1f} °C")
        
        error = setpoint - temp
        self.error_display.setText(f"{error:+.2f} °C")
        
        if abs(error) < 0.5:
            self.error_display.setStyleSheet("color: #00AA00;")  # Zielony
        elif error > 0:
            self.error_display.setStyleSheet("color: #0000AA;")  # Niebieski (za zimno)
        else:
            self.error_display.setStyleSheet("color: #AA0000;")  # Czerwony (za ciepło)
        
        if abs(error) < 1.0:
            self.temp_label.setStyleSheet("color: #00AA00;")
        else:
            self.temp_label.setStyleSheet("color: #FF8800;")
        
        # PWM bars
        self.heater_bar.setValue(heater)
        self.fan_bar.setValue(fan)
        
        # Dane do wykresu
        t = timestamp / 1000.0  # ms -> s
        self.time_data.append(t)
        self.temp_data.append(temp)
        self.setpoint_data.append(setpoint)
        self.heater_data.append(heater)
        self.fan_data.append(fan)
        
        if len(self.time_data) > 1:
            time_arr = np.array(self.time_data)
            time_arr = time_arr - time_arr[0]  # Względny czas
            
            self.temp_curve.setData(time_arr, np.array(self.temp_data))
            self.setpoint_curve.setData(time_arr, np.array(self.setpoint_data))
            self.heater_curve.setData(time_arr, np.array(self.heater_data))
            self.fan_curve.setData(time_arr, np.array(self.fan_data))
    
    def on_ack_received(self, cmd: str):
        """Obsługa ACK"""
        self.statusBar.showMessage(f"OK: {cmd}", 2000)
    
    def on_error_received(self, error: str):
        """Obsługa błędu"""
        self.statusBar.showMessage(f"Błąd: {error}", 3000)
    
    def send_ping(self):
        """Wyślij PING"""
        self.serial_thread.send_command("$PING")
    
    def set_temperature(self):
        """Ustaw nowy setpoint"""
        temp = self.setpoint_spin.value()
        self.serial_thread.send_command(f"$SETP,{temp:.1f}")
    
    def send_start(self):
        """Wyślij START"""
        self.serial_thread.send_command("$START")
    
    def send_stop(self):
        """Wyślij STOP"""
        self.serial_thread.send_command("$STOP")
    
    def closeEvent(self, event):
        """Obsługa zamknięcia okna"""
        self.serial_thread.disconnect()
        self.serial_thread.wait()
        event.accept()



def main():
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    
    window = TemperatureMonitor()
    window.show()
    
    sys.exit(app.exec())


if __name__ == "__main__":
    main()