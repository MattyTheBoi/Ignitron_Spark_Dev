#!/usr/bin/env python3
"""
Spark Amp Test GUI

Simple GUI application to test ESP32 connectivity to Spark amps
via serial commands without physical buttons.

Requirements:
    pip install pyserial

Usage:
    python spark_test_gui.py
"""

import tkinter as tk
from tkinter import ttk, scrolledtext
import serial
import serial.tools.list_ports
import threading
import queue
import time


class SparkTestGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Spark Amp Test Controller")
        self.root.geometry("700x600")

        # Serial connection
        self.ser = None
        self.connected = False
        self.read_thread = None
        self.running = False

        # Queue for thread-safe GUI updates
        self.msg_queue = queue.Queue()

        # Build GUI
        self.create_widgets()

        # Start message processing
        self.root.after(100, self.process_messages)

    def create_widgets(self):
        # Connection Frame
        conn_frame = ttk.LabelFrame(self.root, text="Connection", padding=10)
        conn_frame.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(conn_frame, text="Serial Port:").grid(row=0, column=0, padx=5)

        self.port_combo = ttk.Combobox(conn_frame, width=30, state="readonly")
        self.port_combo.grid(row=0, column=1, padx=5)

        ttk.Button(conn_frame, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=5)

        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=3, padx=5)

        self.status_label = ttk.Label(conn_frame, text="Disconnected", foreground="red")
        self.status_label.grid(row=0, column=4, padx=5)

        # Preset Control Frame
        preset_frame = ttk.LabelFrame(self.root, text="Preset Control", padding=10)
        preset_frame.pack(fill=tk.X, padx=10, pady=5)

        # Preset buttons
        preset_btn_frame = tk.Frame(preset_frame)
        preset_btn_frame.pack()

        for i in range(1, 5):
            btn = ttk.Button(
                preset_btn_frame,
                text=f"Preset {i}",
                width=15,
                command=lambda x=i: self.send_command(f"p{x}")
            )
            btn.grid(row=0, column=i-1, padx=5, pady=5)

        # Bank buttons
        bank_btn_frame = tk.Frame(preset_frame)
        bank_btn_frame.pack(pady=5)

        # Direct bank selection buttons (A, B, C, D, etc.)
        bank_select_frame = tk.Frame(bank_btn_frame)
        bank_select_frame.grid(row=0, column=0, columnspan=4, pady=5)

        ttk.Label(bank_select_frame, text="Select Bank:").pack(side=tk.LEFT, padx=5)

        # Create buttons for banks A through D (0-3)
        bank_labels = ['A', 'B', 'C', 'D']
        for i, label in enumerate(bank_labels):
            btn = ttk.Button(
                bank_select_frame,
                text=f"Bank {label}",
                width=10,
                command=lambda x=i: self.send_command(f"bank{x}")
            )
            btn.pack(side=tk.LEFT, padx=2)

        # Navigation buttons (for banks beyond D)
        bank_nav_frame = tk.Frame(bank_btn_frame)
        bank_nav_frame.grid(row=1, column=0, columnspan=4, pady=5)

        ttk.Button(
            bank_nav_frame,
            text="◄ Previous Bank",
            width=15,
            command=lambda: self.send_command("b-")
        ).pack(side=tk.LEFT, padx=5)

        ttk.Button(
            bank_nav_frame,
            text="Next Bank ►",
            width=15,
            command=lambda: self.send_command("b+")
        ).pack(side=tk.LEFT, padx=5)

        # Effect Control Frame
        fx_frame = ttk.LabelFrame(self.root, text="Effect Control", padding=10)
        fx_frame.pack(fill=tk.X, padx=10, pady=5)

        fx_labels = [
            "Noise Gate", "Compressor", "Drive", "Amp",
            "Modulation", "Delay", "Reverb"
        ]

        fx_btn_frame = tk.Frame(fx_frame)
        fx_btn_frame.pack()

        for i, label in enumerate(fx_labels, 1):
            btn = ttk.Button(
                fx_btn_frame,
                text=label,
                width=12,
                command=lambda x=i: self.send_command(f"fx{x}")
            )
            row = (i - 1) // 4
            col = (i - 1) % 4
            btn.grid(row=row, column=col, padx=3, pady=3)

        # Mode Control
        mode_frame = tk.Frame(fx_frame)
        mode_frame.pack(pady=5)

        ttk.Button(
            mode_frame,
            text="Toggle Mode (Preset/FX)",
            width=25,
            command=lambda: self.send_command("mode")
        ).pack()

        # Info Frame
        info_frame = ttk.LabelFrame(self.root, text="Information", padding=10)
        info_frame.pack(fill=tk.X, padx=10, pady=5)

        info_btn_frame = tk.Frame(info_frame)
        info_btn_frame.pack()

        ttk.Button(
            info_btn_frame,
            text="Status",
            width=15,
            command=lambda: self.send_command("status")
        ).grid(row=0, column=0, padx=5)

        ttk.Button(
            info_btn_frame,
            text="Help",
            width=15,
            command=lambda: self.send_command("help")
        ).grid(row=0, column=1, padx=5)

        ttk.Button(
            info_btn_frame,
            text="Clear Log",
            width=15,
            command=self.clear_log
        ).grid(row=0, column=2, padx=5)

        # Log Frame
        log_frame = ttk.LabelFrame(self.root, text="Serial Output", padding=10)
        log_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.log_text = scrolledtext.ScrolledText(
            log_frame,
            height=10,
            wrap=tk.WORD,
            font=("Courier", 9)
        )
        self.log_text.pack(fill=tk.BOTH, expand=True)

        # Now that log_text exists, we can refresh ports
        self.refresh_ports()

    def refresh_ports(self):
        """Refresh available serial ports"""
        ports = serial.tools.list_ports.comports()
        port_list = [port.device for port in ports]

        self.port_combo['values'] = port_list
        if port_list:
            self.port_combo.current(0)

        self.log(f"Found {len(port_list)} serial port(s)")

    def toggle_connection(self):
        """Connect or disconnect from serial port"""
        if not self.connected:
            self.connect()
        else:
            self.disconnect()

    def connect(self):
        """Connect to selected serial port"""
        port = self.port_combo.get()

        if not port:
            self.log("ERROR: No port selected", "red")
            return

        try:
            self.ser = serial.Serial(port, 115200, timeout=0.1)
            time.sleep(2)  # Wait for ESP32 to reset

            self.connected = True
            self.running = True

            # Start read thread
            self.read_thread = threading.Thread(target=self.read_serial, daemon=True)
            self.read_thread.start()

            # Update GUI
            self.connect_btn.config(text="Disconnect")
            self.status_label.config(text="Connected", foreground="green")
            self.port_combo.config(state="disabled")

            self.log(f"Connected to {port}", "green")

            # Request initial status
            time.sleep(0.5)
            self.send_command("status")

        except Exception as e:
            self.log(f"ERROR: Failed to connect - {e}", "red")
            self.connected = False
            if self.ser:
                try:
                    self.ser.close()
                except:
                    pass
                self.ser = None

    def disconnect(self):
        """Disconnect from serial port"""
        self.running = False

        if self.read_thread:
            self.read_thread.join(timeout=1)

        if self.ser:
            try:
                self.ser.close()
            except:
                pass
            self.ser = None

        self.connected = False

        # Update GUI
        self.connect_btn.config(text="Connect")
        self.status_label.config(text="Disconnected", foreground="red")
        self.port_combo.config(state="readonly")

        self.log("Disconnected", "orange")

    def send_command(self, cmd):
        """Send command to ESP32"""
        if not self.connected or not self.ser:
            self.log("ERROR: Not connected", "red")
            return

        try:
            self.ser.write((cmd + "\n").encode())
            self.log(f"SENT: {cmd}", "blue")
        except Exception as e:
            self.log(f"ERROR: Failed to send - {e}", "red")

    def read_serial(self):
        """Read from serial port in background thread"""
        while self.running and self.ser:
            try:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.msg_queue.put(('recv', line))
            except Exception as e:
                if self.running:
                    self.msg_queue.put(('error', f"Read error: {e}"))
                break

            time.sleep(0.01)

    def process_messages(self):
        """Process messages from queue (runs in GUI thread)"""
        try:
            while True:
                msg_type, msg = self.msg_queue.get_nowait()

                if msg_type == 'recv':
                    self.log(msg, "black")
                elif msg_type == 'error':
                    self.log(msg, "red")

        except queue.Empty:
            pass

        # Schedule next check
        self.root.after(100, self.process_messages)

    def log(self, message, color="black"):
        """Add message to log"""
        self.log_text.tag_config(color, foreground=color)
        self.log_text.insert(tk.END, message + "\n", color)
        self.log_text.see(tk.END)

    def clear_log(self):
        """Clear the log"""
        self.log_text.delete(1.0, tk.END)

    def on_closing(self):
        """Handle window close"""
        if self.connected:
            self.disconnect()
        self.root.destroy()


def main():
    root = tk.Tk()
    app = SparkTestGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()


if __name__ == "__main__":
    main()
