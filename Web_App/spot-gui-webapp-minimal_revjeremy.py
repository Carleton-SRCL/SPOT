"""
ProxiPy Web Interface - Refactored Class-Based Implementation

This application provides a web interface for controlling hardware interfaces
and running simulations for the ProxiPy system. It includes functionality for:
- SSH connections to multiple hardware interfaces
- GPIO pin control on remote Jetson devices
- Hardware thruster checks
- Simulation management
- Data visualization
"""

import os
import sys
import threading
import subprocess
import numpy as np
import plotly.graph_objects as go
import dash
from dash import dcc, html, Input, Output, State, ctx
import dash_bootstrap_components as dbc
from flask import send_file
import io
import matplotlib.pyplot as plt
import paramiko
import time
import socket
import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import tkinter as tk
from tkinter import filedialog
import struct

class SpacecraftAnimator:
    """
    Create animations of spacecraft positions from logged data.
    
    This class generates interactive Plotly animations showing the movement
    of spacecraft based on position and rotation data.
    """
    
    def __init__(self):
        """Initialize the spacecraft animator."""
        self.data = None
        self.time = None
        self.chaser_x = None
        self.chaser_y = None
        self.chaser_rot = None
        self.target_x = None
        self.target_y = None
        self.target_rot = None
        self.obstacle_x = None
        self.obstacle_y = None
        self.obstacle_rot = None
        self.n_frames = 0
        self.spacecraft_size = 0.3
        self.max_frames = 200
        self.skip = 1
        self.effective_frames = 0
        
    # In the SpacecraftAnimator.load_data method, improve error handling:
    def load_data(self, data):
        """
        Load and process spacecraft data.
        
        Args:
            data (dict): Dictionary containing spacecraft position and rotation data
            
        Returns:
            bool: True if data loaded successfully, False otherwise
        """
        try:
            self.data = data
            
            # Check if data is a dictionary
            if not isinstance(data, dict):
                print("Error: Data must be a dictionary")
                return False
                
            # Determine the number of frames
            if 'Time (s)' not in self.data:
                print("Error: Missing 'Time (s)' key in data")
                return False
                
            # Check for required position keys
            required_keys = ['Chaser Px (m)', 'Chaser Py (m)', 'Target Px (m)', 'Target Py (m)']
            missing_keys = [key for key in required_keys if key not in self.data]
            if missing_keys:
                print(f"Error: Missing required keys: {', '.join(missing_keys)}")
                return False
                
            self.n_frames = len(self.data['Time (s)'])
            
            # For performance, subsample frames if there are too many
            self.skip = max(1, self.n_frames // self.max_frames)
            self.effective_frames = self.n_frames // self.skip
            
            self._extract_positions()
            return True
        except Exception as e:
            print(f"Error loading animation data: {e}")
            return False
    
    def _extract_positions(self):
        """
        Extract position and rotation data for spacecraft, subsampling if needed.
        """
        # Extract all data first
        self.time = self.data['Time (s)']
        
        required_keys = ['Chaser Px (m)', 'Chaser Py (m)', 'Target Px (m)', 'Target Py (m)']
        if not all(key in self.data for key in required_keys):
            raise ValueError("Missing required position data for chaser or target")
            
        self.chaser_x = self.data['Chaser Px (m)']
        self.chaser_y = self.data['Chaser Py (m)']
        self.target_x = self.data['Target Px (m)']
        self.target_y = self.data['Target Py (m)']
        
        # Extract rotation data if available
        self.chaser_rot = self.data.get('Chaser Rz (rad)', np.zeros_like(self.time))
        self.target_rot = self.data.get('Target Rz (rad)', np.zeros_like(self.time))
        
        # Check if obstacle data exists
        if 'Obstacle Px (m)' in self.data and 'Obstacle Py (m)' in self.data:
            self.obstacle_x = self.data['Obstacle Px (m)']
            self.obstacle_y = self.data['Obstacle Py (m)']
            self.obstacle_rot = self.data.get('Obstacle Rz (rad)', np.zeros_like(self.time))
        else:
            self.obstacle_x = None
            self.obstacle_y = None
            self.obstacle_rot = None
            
        # Subsample data for performance
        if self.skip > 1:
            self.time = self.time[::self.skip]
            self.chaser_x = self.chaser_x[::self.skip]
            self.chaser_y = self.chaser_y[::self.skip]
            self.chaser_rot = self.chaser_rot[::self.skip]
            self.target_x = self.target_x[::self.skip]
            self.target_y = self.target_y[::self.skip]
            self.target_rot = self.target_rot[::self.skip]
            if self.obstacle_x is not None:
                self.obstacle_x = self.obstacle_x[::self.skip]
                self.obstacle_y = self.obstacle_y[::self.skip]
                self.obstacle_rot = self.obstacle_rot[::self.skip]

    def _create_square_shape(self, x, y, rotation, size, color):
        """
        Create a rotated square shape representing a spacecraft.
        
        Args:
            x, y (float): Center coordinates of the square
            rotation (float): Rotation angle in radians
            size (float): Side length of the square in meters
            color (str): Fill color of the square
            
        Returns:
            numpy.ndarray: Array of (x, y) coordinates for the rotated square
        """
        # Create square corners (centered at origin)
        half_size = size / 2
        corners = np.array([
            [-half_size, -half_size],
            [half_size, -half_size],
            [half_size, half_size],
            [-half_size, half_size],
            [-half_size, -half_size]  # Close the shape
        ])
        
        # Rotation matrix
        rot_matrix = np.array([
            [np.cos(rotation), -np.sin(rotation)],
            [np.sin(rotation), np.cos(rotation)]
        ])
        
        # Apply rotation and translation
        rotated_corners = np.dot(corners, rot_matrix.T)
        rotated_corners[:, 0] += x
        rotated_corners[:, 1] += y
        
        return rotated_corners

    def create_animation(self, use_dark_theme=True):
        """
        Create an animated Plotly figure showing spacecraft trajectories.
        
        Args:
            use_dark_theme (bool): Whether to use dark theme consistent with the web app
            
        Returns:
            plotly.graph_objects.Figure: The animated figure, or None if no data loaded
        """
        if self.data is None or self.time is None:
            return None
            
        # Fixed ranges for the animation viewport
        x_min, x_max = -0.5, 4.0
        y_min, y_max = -0.5, 2.9
        
        # Create the base figure with appropriate template
        template = "plotly_dark" if use_dark_theme else "plotly_white"
        bg_color = '#333333' if use_dark_theme else 'white'
        text_color = 'white' if use_dark_theme else 'black'
        
        fig = make_subplots()

        # Create workspace rectangle
        rect_corners = np.array([
            [0, 0],
            [3.4, 0],
            [3.4, 2.5],
            [0, 2.5],
            [0, 0]
        ])

        # Background rectangle color based on theme
        workspace_fill = 'rgba(100, 100, 100, 0.2)' if use_dark_theme else 'rgba(200, 200, 200, 0.2)'
        workspace_line = 'white' if use_dark_theme else 'black'

        fig.add_trace(go.Scatter(
            x=rect_corners[:, 0], y=rect_corners[:, 1],
            fill="toself",
            fillcolor=workspace_fill,
            line=dict(color=workspace_line, width=2),
            name='Workspace',
            showlegend=False
        ))

        # X-axis (red arrow)
        fig.add_trace(go.Scatter(
            x=[0, 0.3],
            y=[0, 0],
            mode='lines+markers',
            line=dict(color='red', width=3),
            marker=dict(size=[0, 15], symbol='arrow', angleref='previous'),
            name='X-axis',
            hoverinfo='none'
        ))

        # Y-axis (green arrow)
        fig.add_trace(go.Scatter(
            x=[0, 0],
            y=[0, 0.3],
            mode='lines+markers',
            line=dict(color='green', width=3),
            marker=dict(size=[0, 15], symbol='arrow', angleref='previous'),
            name='Y-axis',
            hoverinfo='none'
        ))
        
        # Create initial paths
        # Chaser path
        fig.add_trace(go.Scatter(
            x=[self.chaser_x[0]], y=[self.chaser_y[0]],
            mode='lines',
            line=dict(color='rgba(255, 0, 0, 0.7)', width=2),
            name='Chaser Path'
        ))
        
        # Target path
        fig.add_trace(go.Scatter(
            x=[self.target_x[0]], y=[self.target_y[0]],
            mode='lines',
            line=dict(color='rgba(120, 120, 120, 0.7)', width=2),
            name='Target Path'
        ))
        
        # Obstacle path (if available)
        if self.obstacle_x is not None:
            fig.add_trace(go.Scatter(
                x=[self.obstacle_x[0]], y=[self.obstacle_y[0]],
                mode='lines',
                line=dict(color='rgba(0, 0, 255, 0.7)', width=2),
                name='Obstacle Path'
            ))
        
        # Create initial spacecraft shapes
        # Chaser spacecraft (red)
        chaser_corners = self._create_square_shape(
            self.chaser_x[0], self.chaser_y[0], 
            self.chaser_rot[0], self.spacecraft_size, 'red'
        )
        fig.add_trace(go.Scatter(
            x=chaser_corners[:, 0], y=chaser_corners[:, 1],
            fill="toself",
            fillcolor='rgba(255, 0, 0, 0.5)',
            line=dict(color=workspace_line, width=2),
            name='Chaser',
            showlegend=False
        ))
        
        # Target spacecraft (black/gray depending on theme)
        target_fill = 'rgba(100, 100, 100, 0.5)' if use_dark_theme else 'rgba(0, 0, 0, 0.5)'
        target_corners = self._create_square_shape(
            self.target_x[0], self.target_y[0], 
            self.target_rot[0], self.spacecraft_size, 'black'
        )
        fig.add_trace(go.Scatter(
            x=target_corners[:, 0], y=target_corners[:, 1],
            fill="toself",
            fillcolor=target_fill,
            line=dict(color=workspace_line, width=2),
            name='Target',
            showlegend=False
        ))
        
        # Obstacle spacecraft (if available)
        if self.obstacle_x is not None:
            obstacle_corners = self._create_square_shape(
                self.obstacle_x[0], self.obstacle_y[0], 
                self.obstacle_rot[0], self.spacecraft_size, 'blue'
            )
            fig.add_trace(go.Scatter(
                x=obstacle_corners[:, 0], y=obstacle_corners[:, 1],
                fill="toself",
                fillcolor='rgba(0, 0, 255, 0.5)',
                line=dict(color=workspace_line, width=2),
                name='Obstacle',
                showlegend=False
            ))
        
        # Replace the update_layout call with this:
        fig.update_layout(
            title={
                'text': "Spacecraft Trajectories",
                'y': 0.95,
                'x': 0.5,
                'xanchor': 'center',
                'yanchor': 'top',
                'font': {'size': 20, 'color': text_color}
            },
            xaxis={
                'range': [x_min, x_max], 
                'title': {
                    'text': "X (m)",
                    'font': {'size': 16, 'color': text_color},
                    'standoff': 10
                },
                'tickfont': {'size': 12, 'color': text_color},
                'gridcolor': 'rgba(255, 255, 255, 0.2)' if use_dark_theme else 'rgba(0, 0, 0, 0.1)'
            },
            yaxis={
                'range': [y_min, y_max], 
                'title': {
                    'text': "Y (m)",
                    'font': {'size': 16, 'color': text_color},
                    'standoff': 10
                },
                'tickfont': {'size': 12, 'color': text_color},
                'scaleanchor': 'x',
                'scaleratio': 1,
                'gridcolor': 'rgba(255, 255, 255, 0.2)' if use_dark_theme else 'rgba(0, 0, 0, 0.1)'
            },
            plot_bgcolor=bg_color,
            paper_bgcolor=bg_color,
            autosize=True,  # Enable autosize instead of fixed width/height
            margin={'l': 50, 'r': 50, 't': 80, 'b': 100},  # Reduced margins
            hovermode="closest",
            legend={
                'x': 0.5,
                'y': -0.15,  # Moved up for better spacing
                'xanchor': 'center',
                'yanchor': 'top',
                'orientation': 'h',
                'font': {'color': text_color}
            },
            updatemenus=[{
                "buttons": [
                    {
                        "args": [None, {"frame": {"duration": 50, "redraw": False}, "fromcurrent": True}],
                        "label": "Play",
                        "method": "animate"
                    },
                    {
                        "args": [[None], {"frame": {"duration": 0, "redraw": False}, "mode": "immediate"}],
                        "label": "Pause",
                        "method": "animate"
                    }
                ],
                "direction": "left",
                "pad": {"r": 10, "t": 10},
                "showactive": True,  # Show active button state
                "type": "buttons",
                "x": 0.1,
                "xanchor": "right",
                "y": 0,  # Adjusted position
                "yanchor": "top",
                "font": {'color': text_color},
                "bgcolor": 'rgba(100, 100, 100, 0.3)' if use_dark_theme else 'rgba(0, 0, 0, 0.1)'
            }],
            template=template
        )
        
        # Create frames
        frames = []
        for i in range(len(self.time)):
            frame_data = []

            # Add background rectangle as first element in each frame
            frame_data.append(go.Scatter(
                x=rect_corners[:, 0], y=rect_corners[:, 1],
                fill="toself",
                fillcolor=workspace_fill,
                line=dict(color=workspace_line, width=2),
                showlegend=False
            ))

            # X-axis (red arrow)
            frame_data.append(go.Scatter(
                x=[0, 0.3],
                y=[0, 0],
                mode='lines+markers',
                line=dict(color='red', width=3),
                marker=dict(size=[0, 15], symbol='arrow', angleref='previous'),
                hoverinfo='none'
            ))

            # Y-axis (green arrow)
            frame_data.append(go.Scatter(
                x=[0, 0],
                y=[0, 0.3],
                mode='lines+markers',
                line=dict(color='green', width=3),
                marker=dict(size=[0, 15], symbol='arrow', angleref='previous'),
                hoverinfo='none'
            ))
            
            # Chaser path
            frame_data.append(go.Scatter(
                x=self.chaser_x[:i+1],
                y=self.chaser_y[:i+1],
                mode='lines',
                line=dict(color='rgba(255, 0, 0, 0.7)', width=2, dash='dot')
            ))
            
            # Target path
            frame_data.append(go.Scatter(
                x=self.target_x[:i+1],
                y=self.target_y[:i+1],
                mode='lines',
                line=dict(color='rgba(120, 120, 120, 0.7)' if use_dark_theme else 'rgba(0, 0, 0, 0.7)', width=2)
            ))
            
            # Obstacle path (if available)
            if self.obstacle_x is not None:
                frame_data.append(go.Scatter(
                    x=self.obstacle_x[:i+1],
                    y=self.obstacle_y[:i+1],
                    mode='lines',
                    line=dict(color='rgba(0, 0, 255, 0.7)', width=2)
                ))
            
            # Rotated spacecraft shapes
            # Chaser spacecraft
            chaser_corners = self._create_square_shape(
                self.chaser_x[i], self.chaser_y[i], 
                self.chaser_rot[i], self.spacecraft_size, 'red'
            )
            frame_data.append(go.Scatter(
                x=chaser_corners[:, 0], y=chaser_corners[:, 1],
                fill="toself",
                fillcolor='rgba(255, 0, 0, 0.5)',
                line=dict(color=workspace_line, width=2),
                showlegend=False
            ))
            
            # Target spacecraft
            target_corners = self._create_square_shape(
                self.target_x[i], self.target_y[i], 
                self.target_rot[i], self.spacecraft_size, 'black'
            )
            frame_data.append(go.Scatter(
                x=target_corners[:, 0], y=target_corners[:, 1],
                fill="toself",
                fillcolor=target_fill,
                line=dict(color=workspace_line, width=2),
                showlegend=False
            ))
            
            # Obstacle spacecraft (if available)
            if self.obstacle_x is not None:
                obstacle_corners = self._create_square_shape(
                    self.obstacle_x[i], self.obstacle_y[i], 
                    self.obstacle_rot[i], self.spacecraft_size, 'blue'
                )
                frame_data.append(go.Scatter(
                    x=obstacle_corners[:, 0], y=obstacle_corners[:, 1],
                    fill="toself",
                    fillcolor='rgba(0, 0, 255, 0.5)',
                    line=dict(color=workspace_line, width=2),
                    showlegend=False
                ))
                
            frames.append(go.Frame(data=frame_data, name=str(i)))
        
        fig.frames = frames
        
        # Create a more efficient slider - only show key frames
        slider_steps = []
        step_size = max(1, len(frames) // 20)  # Show at most 20 slider steps
        
        for i in range(0, len(frames), step_size):
            step = {
                "args": [
                    [str(i)],
                    {"frame": {"duration": 50, "redraw": False}, "mode": "immediate"}
                ],
                "label": f"{self.time[i]:.1f}",
                "method": "animate"
            }
            slider_steps.append(step)
            
        # Update the slider layout with better spacing
        fig.update_layout(
            sliders=[{
                "active": 0,
                "steps": slider_steps,
                "x": 0.5,
                "y": 0,  # Adjusted position
                "xanchor": "center",
                "yanchor": "top",
                "currentvalue": {
                    "font": {"size": 14, "color": text_color},
                    "prefix": "Time: ",
                    "suffix": " s",
                    "visible": True,
                    "xanchor": "center"
                },
                "len": 0.7,
                "pad": {"b": 10, "t": 30},  # Better spacing
                "ticklen": 10,  # Longer tick marks for better visibility
                "tickwidth": 2,  # Thicker tick marks
                "bgcolor": 'rgba(100, 100, 100, 0.3)' if use_dark_theme else 'rgba(0, 0, 0, 0.1)'
            }]
        )
        
        return fig

class SSHConnectionManager:
    """
    Manages SSH connections to remote devices.
    
    Handles establishing, monitoring, and testing SSH connections
    to remote hardware interfaces.
    """
    def __init__(self):
        """Initialize the SSH connection manager with default connection status."""
        # Initialize SSH connection status for each interface
        self.ssh_connections = {
            interface: {
                "connected": False,
                "checking": False,
                "last_check": 0,
                "host": None,
                "port": None,
                "username": None,
                "password": None,
                "key_file": None
            } for interface in ["chaser", "target", "obstacle"]
        }
    
    def test_ssh_connection(self, host, port, username, password=None, key_file=None):
        """
        Test if an SSH connection can be established.
        Uses a short timeout to quickly detect if a host is unreachable.
        
        Args:
            host (str): Hostname or IP address
            port (int): SSH port
            username (str): SSH username
            password (str, optional): SSH password
            key_file (str, optional): Path to SSH private key file
            
        Returns:
            bool: True if connection successful, False otherwise
        """
        try:
            # Try to connect to the SSH port with a very short timeout
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)  # 1 second timeout for quick checking
            # Ensure port is converted to integer
            port_num = int(port)
            result = sock.connect_ex((host, port_num))
            sock.close()
            
            if result != 0:
                print(f"SSH port test failed for {host}:{port}")
                return False  # Port is not open
            
            # If port is open, try to establish SSH connection with a short timeout
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            
            try:
                # Try to connect with a short timeout
                if password:
                    client.connect(host, port=port_num, username=username, password=password, timeout=2, banner_timeout=2)
                elif key_file:
                    key = paramiko.RSAKey.from_private_key_file(key_file)
                    client.connect(host, port=port_num, username=username, pkey=key, timeout=2, banner_timeout=2)
                else:
                    return False  # No authentication provided
                
                # If we get here, connection was successful
                client.close()
                return True
                
            except (socket.error, paramiko.SSHException, paramiko.ssh_exception.NoValidConnectionsError, 
                    TimeoutError, ConnectionRefusedError, ConnectionResetError, Exception) as e:
                print(f"SSH connection attempt failed: {e}")
                return False
            
        except Exception as e:
            print(f"Socket connection test failed: {e}")
            return False
    
    def start_monitoring(self, interface, host, port, username, password=None, key_file=None):
        """
        Start monitoring a connection to a specific interface.
        
        Args:
            interface (str): The interface to monitor
            host (str): Host to connect to
            port (int): Port to connect to
            username (str): Username for SSH
            password (str, optional): Password for SSH
            key_file (str, optional): Key file for SSH
            
        Returns:
            bool: True if monitoring started successfully, False otherwise
        """
        if self.test_ssh_connection(host, port, username, password, key_file):
            # Store the connection parameters for background checking
            self.ssh_connections[interface]["connected"] = True
            self.ssh_connections[interface]["checking"] = True
            self.ssh_connections[interface]["last_check"] = time.time()
            self.ssh_connections[interface]["host"] = host
            self.ssh_connections[interface]["port"] = port
            self.ssh_connections[interface]["username"] = username
            self.ssh_connections[interface]["password"] = password
            self.ssh_connections[interface]["key_file"] = key_file
            
            # Start a thread for checking the connection status
            threading.Thread(target=self.check_connection_status, args=[interface], daemon=True).start()
            return True
        return False
    
    def check_connection_status(self, interface):
        """
        Continuously check the SSH connection status in the background.
        
        Args:
            interface (str): The interface to check connection status for
        """
        connection_info = self.ssh_connections[interface]
        if connection_info["checking"]:
            # Get connection parameters from our stored data
            host = connection_info["host"]
            port = connection_info["port"]
            username = connection_info["username"]
            password = connection_info["password"]
            key_file = connection_info["key_file"]
            
            # Test the connection
            was_connected = connection_info["connected"]
            is_connected = self.test_ssh_connection(host, port, username, password, key_file)
            connection_info["connected"] = is_connected
            connection_info["last_check"] = time.time()
            
            # If the connection was lost, stop checking
            if was_connected and not is_connected:
                connection_info["checking"] = False
                print(f"Connection to {interface} was lost!")
            
            # Schedule the next check in 5 seconds
            if connection_info["checking"]:
                threading.Timer(5, self.check_connection_status, args=[interface]).start()
    
    def stop_monitoring(self, interface):
        """
        Stop monitoring a connection to a specific interface.
        
        Args:
            interface (str): The interface to stop monitoring
        """
        self.ssh_connections[interface]["connected"] = False
        self.ssh_connections[interface]["checking"] = False
    
    def is_connected(self, interface):
        """
        Check if an interface is currently connected.
        
        Args:
            interface (str): The interface to check
            
        Returns:
            bool: True if connected, False otherwise
        """
        return self.ssh_connections[interface]["connected"]

class HardwareController:
    """
    Controls hardware via SSH connections.
    
    """
    def __init__(self, ssh_manager):
        """
        Initialize the hardware controller.
        
        Args:
            ssh_manager (SSHConnectionManager): Manager for SSH connections
        """
        self.ssh_manager = ssh_manager
        # Thruster check status dictionaries for each interface
        self.thruster_check_status = {
            interface: {
                "running": False,
                "progress": 0,
                "thrusters": [1, 2, 3, 4, 5, 6, 7, 8],
                "current_thruster": -1,
                "message": "",
                "completed": False,
                "failed_pins": []
            } for interface in ["chaser", "target", "obstacle"]
        }

        self.air_bearing = 0

    def set_thruster_airbrg(self, thruster, air_brg, host, username, password=None, key_file=None, port=22):
        if len(thruster) != 8:
            print("Error: Length of thruster must be 8")
        
        # Create SSH client
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        
        try:
            # Connect to the Jetson
            if password:
                client.connect(host, port=port, username=username, password=password)
            elif key_file:
                key = paramiko.RSAKey.from_private_key_file(key_file)
                client.connect(host, port=port, username=username, pkey=key)
            else:
                print("Error: Either password or key_file must be provided")
                return False
        
            packet = self.generate_packet(thruster, air_brg)

            command = f'echo srcl2023 | sudo -S bash -c "echo {"".join(f"\\x{byte:02x}" for byte in packet)} | xxd -r -p > /dev/ttyACM0"'
            # Execute the command
            stdin, stdout, stderr = client.exec_command(command)
            
            # Get the output
            output = stdout.read().decode().strip()
            print(f"SSH Output: {output}")
            
            # Check for actual errors (not warnings)
            err = stderr.read().decode()
            if err and "Traceback" in err:  # Only print actual errors, not warnings
                print(f"Error executing command: {err}")
                return False
            
            return True

        except Exception as e:
            print(f"SSH connection error: {e}")
            return False
        
        finally:
            # Always close the connection
            client.close()

    def run_thruster_check(self, interface):
        """
        Executes a thruster check for a specific interface.
        This function activates each pin in sequence, then deactivates it.
        
        Args:
            interface (str): The interface to run the thruster check on
        """
        status = self.thruster_check_status[interface]
        
        try:
            pins = status["thrusters"]
            duration = status["duration"]
            host = status["host"]
            port = status["port"]
            username = status["username"]
            password = status["password"]
            key_file = status["key_file"]

            thrusters = [0,0,0,0,0,0,0,0]
            self.air_bearing = 0

            self.set_thruster_airbrg(thrusters, self.air_bearing, host, username, password, key_file, port)

            for i, pin in enumerate(pins):
                # Update current pin
                status["current_thruster"] = i
                
                thrusters = [0,0,0,0,0,0,0,0]
                thrusters[i] = 100

                # Turn pin ON
                success = self.set_thruster_airbrg(thrusters, self.air_bearing, host, username, password, key_file, port)
                if not success:
                    status["failed_pins"].append(pin)
                    continue
                
                # Wait for specified duration
                time.sleep(duration)

        except Exception as e:
            status["message"] = f"Error: {str(e)}"
            status["failed_pins"] = pins[status["current_thruster"]:]
        
        finally:
            thr = [0,0,0,0,0,0,0,0]
            air_brg = 0

            success = self.set_thruster_airbrg(thr, air_brg, host, username, password, key_file, port)

            # Mark as completed
            status["running"] = False
            status["completed"] = True

    def start_thruster_check(self, interface, duration, host, port, username, password=None, key_file=None):
        """
        Start a thruster check for a specific interface.
        
        Args:
            interface (str): The interface to run the thruster check on
            duration (float): Duration to keep each pin activated
            host (str): Host to connect to
            port (int): Port to connect to
            username (str): Username for SSH
            password (str, optional): Password for SSH
            key_file (str, optional): Key file for SSH
        
        Returns:
            bool: True if the thruster check was started, False otherwise
        """
        if not self.ssh_manager.is_connected(interface):
            return False
        
        # Reset status
        self.thruster_check_status[interface] = {
            "running": True,
            "progress": 0,
            "thrusters": [1, 2, 3, 4, 5, 6, 7, 8],
            "current_thruster": -1,
            "duration": duration,
            "host": host,
            "port": port,
            "username": username,
            "password": password,
            "key_file": key_file,
            "message": "Starting thruster check...",
            "completed": False,
            "failed_pins": []
        }
        
        # Start the thruster check in a separate thread
        threading.Thread(target=self.run_thruster_check, args=[interface], daemon=True).start()
        return True

    def generate_packet(self, thrusters, air_bearing):
        if len(thrusters) != 8:
            raise ValueError("thrusters must contain exactly 8 elements (duty cycles)")

        packet = bytearray(37)

        packet[0] = 0xAA  # 170

        for i in range(8):
            dbytes = struct.pack('<f', np.float32(thrusters[i]))

            base = 1 + i * 4

            packet[base:base + 4] = dbytes

        abytes = struct.pack('<f', np.float32(air_bearing))
        packet[33:37] = abytes

        return bytes(packet)

class DataVisualizer:
    """
    Handles data loading, visualization, and export.
    
    Manages loading and visualizing data files, as well as exporting
    plots to PDF.
    """
    def __init__(self):
        """Initialize the data visualizer with empty loaded data."""
        self.loaded_data = {}
    
    def load_data_from_contents(self, contents, filename):
        """
        Load data from uploaded contents.
        
        Args:
            contents (str): File contents as string
            filename (str): Filename
        
        Returns:
            tuple: (success, message, keys)
        """
        if not filename.endswith('.npy'):
            return False, "Please upload a .npy file", None
        
        try:
            # Decode the content
            content_type, content_string = contents.split(',')
            import base64
            import tempfile
            
            # Create a temporary file to save the decoded content
            with tempfile.NamedTemporaryFile(delete=False, suffix='.npy') as temp_file:
                temp_file.write(base64.b64decode(content_string))
                temp_path = temp_file.name
            
            # Load the data from the temporary file
            self.loaded_data = np.load(temp_path, allow_pickle=True).item()
            os.unlink(temp_path)  # Delete the temporary file
            
            if isinstance(self.loaded_data, dict):
                keys = list(self.loaded_data.keys())
                return True, f"Loaded: {filename}", keys
            else:
                return False, "Error: File is not a dictionary", None
        except Exception as e:
            return False, f"Error: {str(e)}", None
    
    def create_empty_plot(self):
        """
        Create an empty plot to display when no data is loaded.
        
        Returns:
            go.Figure: Empty plotly figure
        """
        fig = go.Figure()
        fig.update_layout(
            template="plotly_dark",
            plot_bgcolor='#333333',
            paper_bgcolor='#333333',
            xaxis=dict(
                showgrid=True,
                gridcolor='rgba(255, 255, 255, 0.2)'
            ),
            yaxis=dict(
                showgrid=True,
                gridcolor='rgba(255, 255, 255, 0.2)'
            ),
            margin=dict(l=40, r=40, t=40, b=40),
            height=550
        )
        # Add a text annotation to the empty plot
        fig.add_annotation(
            text="No data loaded",
            xref="paper", yref="paper",
            x=0.5, y=0.5,
            showarrow=False,
            font=dict(color="white", size=16)
        )
        return fig
    
    def create_plot(self, x_key, y_key):
        """
        Create a plot using the specified x and y keys.
        
        Args:
            x_key (str): Key for x-axis data
            y_key (str): Key for y-axis data
        
        Returns:
            go.Figure: Plotly figure with the plot
        """
        if not x_key or not y_key or not self.loaded_data:
            return self.create_empty_plot()
        
        x_data = self.loaded_data[x_key]
        y_data = self.loaded_data[y_key]
        
        fig = go.Figure()
        
        if isinstance(y_data, np.ndarray):
            if y_data.ndim == 1:
                if isinstance(x_data, np.ndarray) and x_data.ndim == 1 and len(x_data) == len(y_data):
                    fig.add_trace(go.Scatter(x=x_data, y=y_data, mode='lines', line=dict(color='#5599ff', width=2)))
                else:
                    fig.add_trace(go.Scatter(y=y_data, mode='lines', line=dict(color='#5599ff', width=2)))
                
                fig.update_layout(
                    title=f"{y_key} vs {x_key}",
                    xaxis_title=x_key,
                    yaxis_title=y_key,
                    template="plotly_dark",
                    plot_bgcolor='#333333',
                    paper_bgcolor='#333333',
                    xaxis=dict(
                        showgrid=True,
                        gridcolor='rgba(255, 255, 255, 0.2)'
                    ),
                    yaxis=dict(
                        showgrid=True,
                        gridcolor='rgba(255, 255, 255, 0.2)'
                    )
                )
            elif y_data.ndim == 2:
                fig.add_trace(go.Heatmap(z=y_data, colorscale='Plasma'))
                
                fig.update_layout(
                    title=y_key,
                    xaxis_title='X Dimension',
                    yaxis_title='Y Dimension',
                    template="plotly_dark",
                    plot_bgcolor='#333333',
                    paper_bgcolor='#333333'
                )
            else:
                fig = self.create_empty_plot()
                fig.add_annotation(
                    text=f"Cannot display {y_data.ndim}D array",
                    xref="paper", yref="paper",
                    x=0.5, y=0.5,
                    showarrow=False,
                    font=dict(color="white", size=14)
                )
        else:
            fig = self.create_empty_plot()
            fig.add_annotation(
                text=f"Data type: {type(y_data)}",
                xref="paper", yref="paper",
                x=0.5, y=0.5,
                showarrow=False,
                font=dict(color="white", size=14)
            )
        
        return fig
    
    def export_to_pdf(self, x_key, y_key):
        """
        Export the current plot to a PDF file.
        
        Args:
            x_key (str): Key for x-axis data
            y_key (str): Key for y-axis data
        
        Returns:
            bytes: PDF file as bytes
        """
        if not x_key or not y_key or not self.loaded_data:
            return None
        
        # Create a matplotlib figure for PDF export
        fig, ax = plt.subplots(figsize=(10, 8))
        ax.set_facecolor('#333333')
        fig.patch.set_facecolor('#333333')
        
        x_data = self.loaded_data[x_key]
        y_data = self.loaded_data[y_key]
        
        if isinstance(y_data, np.ndarray):
            if y_data.ndim == 1:
                if isinstance(x_data, np.ndarray) and x_data.ndim == 1 and len(x_data) == len(y_data):
                    ax.plot(x_data, y_data, '-', color='#5599ff', linewidth=1.5)
                else:
                    ax.plot(y_data, '-', color='#5599ff', linewidth=1.5)
                ax.set_title(f"{y_key} vs {x_key}", color='white', fontsize=16)
                ax.set_xlabel(x_key, color='white', fontsize=14)
                ax.set_ylabel(y_key, color='white', fontsize=14)
                ax.grid(True, linestyle='--', alpha=0.6)
                ax.tick_params(colors='white', labelsize=14)
            elif y_data.ndim == 2:
                img = ax.imshow(y_data, cmap='plasma', aspect='auto')
                ax.set_title(y_key, color='white', fontsize=16)
                ax.set_xlabel('X Dimension', color='white', fontsize=14)
                ax.set_ylabel('Y Dimension', color='white', fontsize=14)
                ax.tick_params(colors='white', labelsize=14)
                fig.colorbar(img, ax=ax)
        
        # Save figure to a BytesIO object
        buf = io.BytesIO()
        fig.savefig(buf, format='pdf', bbox_inches='tight')
        buf.seek(0)
        
        # Close the figure to free memory
        plt.close(fig)
        
        return buf.getvalue()

class SimulationManager:
    """
    Manages running simulations.
    
    Handles starting and monitoring simulations.
    """
    def __init__(self):
        """Initialize the simulation manager."""
        self.simulation_running = False
    
    def run_simulation(self):
        """
        Run the main simulation.
        
        Returns:
            str: Status message
        """
        self.simulation_running = True
        
        try:
            # Run main.py located in the "main" directory
            process = subprocess.Popen(
                ["python", "main.py"],
                cwd=os.path.join(os.getcwd(), "main")
            )
            process.wait()
        except Exception as e:
            print("Error running simulation:", e)
        
        self.simulation_running = False
        return "Simulation completed"
    
    def is_simulation_running(self):
        """
        Check if a simulation is currently running.
        
        Returns:
            bool: True if a simulation is running, False otherwise
        """
        return self.simulation_running

class AnimationManager:
    """
    Manages spacecraft animations in the web interface.
    """
    def __init__(self):
        """Initialize the animation manager."""
        self.animator = SpacecraftAnimator()
        self.data = None
        
    def load_animation_data(self, contents, filename):
        """
        Load animation data from uploaded contents.
        
        Args:
            contents (str): File contents as string
            filename (str): Filename
            
        Returns:
            tuple: (success, message)
        """
        if not filename.endswith('.npy'):
            return False, "Please upload a .npy file"
        
        try:
            # Decode the content
            content_type, content_string = contents.split(',')
            import base64
            import tempfile
            
            # Create a temporary file to save the decoded content
            with tempfile.NamedTemporaryFile(delete=False, suffix='.npy') as temp_file:
                temp_file.write(base64.b64decode(content_string))
                temp_path = temp_file.name
            
            # Load the data from the temporary file
            data_loaded = np.load(temp_path, allow_pickle=True)
            if isinstance(data_loaded, np.ndarray) and data_loaded.shape == ():
                self.data = data_loaded.item()
            else:
                self.data = data_loaded
                
            # Clean up the temporary file
            import os
            os.unlink(temp_path)
            
            # Load the data into the animator
            if not self.animator.load_data(self.data):
                return False, "Invalid animation data format"
            
            return True, f"Loaded animation data from: {filename}"
        except Exception as e:
            return False, f"Error loading animation data: {str(e)}"
    
    def create_animation(self):
        """
        Create the spacecraft animation figure.
        
        Returns:
            go.Figure: The animation figure if data is loaded, or an empty figure with controls
        """
        # Create an empty figure with proper controls if no data is loaded
        if self.data is None:
            empty_fig = go.Figure()
            
            # Set up the layout with same dimensions as the real animation
            empty_fig.update_layout(
                title={
                    'text': "Spacecraft Trajectories",
                    'y': 0.95,
                    'x': 0.5,
                    'xanchor': 'center',
                    'yanchor': 'top',
                    'font': {'size': 20, 'color': 'white'}
                },
                xaxis={
                    'range': [-0.5, 4.0],
                    'title': {'text': "X (m)", 'font': {'size': 16, 'color': 'white'}},
                    'gridcolor': 'rgba(255, 255, 255, 0.2)'
                },
                yaxis={
                    'range': [-0.5, 2.9],
                    'title': {'text': "Y (m)", 'font': {'size': 16, 'color': 'white'}},
                    'scaleanchor': 'x',
                    'scaleratio': 1,
                    'gridcolor': 'rgba(255, 255, 255, 0.2)'
                },
                plot_bgcolor='#333333',
                paper_bgcolor='#333333',
                autosize=True,
                height=500,
                margin={'l': 50, 'r': 50, 't': 80, 'b': 100},
                
                # Add empty controls that will be populated when data is loaded
                updatemenus=[{
                    "buttons": [
                        {"label": "Play", "method": "animate", "args": [None, {"frame": {"duration": 50}}]},
                        {"label": "Pause", "method": "animate", "args": [[None], {"mode": "immediate"}]}
                    ],
                    "direction": "left",
                    "showactive": True,
                    "type": "buttons",
                    "x": 0.1,
                    "y": 0,
                    "xanchor": "right",
                    "yanchor": "top",
                    "bgcolor": 'rgba(100, 100, 100, 0.3)'
                }],
                sliders=[{
                    "active": 0,
                    "steps": [{"label": "0.0", "method": "animate", "args": [["0"], {"mode": "immediate"}]}],
                    "x": 0.5,
                    "y": 0,
                    "xanchor": "center",
                    "yanchor": "top",
                    "currentvalue": {
                        "font": {"size": 14, "color": "white"},
                        "prefix": "Time: ",
                        "suffix": " s",
                        "visible": True
                    },
                    "len": 0.7,
                    "pad": {"b": 10, "t": 30},
                    "ticklen": 10,
                    "tickwidth": 2
                }]
            )
            
            empty_fig.add_annotation(
                text="Upload a .npy file with spacecraft data",
                xref="paper", yref="paper",
                x=0.5, y=0.5,
                showarrow=False,
                font=dict(color="white", size=16)
            )
            return empty_fig
        
        # Create animation with dark theme to match app
        return self.animator.create_animation(use_dark_theme=True)

class ProxiPyApp:
    """
    Main ProxiPy web application class.
    
    This class ties together all the components of the ProxiPy web interface.
    """
    def __init__(self):
        """Initialize the ProxiPy application."""
        # Add the project path so that lib files can be read
        project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
        sys.path.append(project_root)
        
        # Initialize component classes
        self.ssh_manager = SSHConnectionManager()
        self.hardware_controller = HardwareController(self.ssh_manager)
        self.data_visualizer = DataVisualizer()
        self.simulation_manager = SimulationManager()
        self.animation_manager = AnimationManager()  # Add this line
        
        # Initialize the Dash application with Bootstrap styling
        self.app = dash.Dash(
            __name__, 
            external_stylesheets=[dbc.themes.DARKLY],
            external_scripts=[
                {"src": "https://cdnjs.cloudflare.com/ajax/libs/jquery/3.6.0/jquery.min.js"}
            ]
        )
        
        # Add custom CSS for dropdowns and default plot theme
        self.app.index_string = '''
        <!DOCTYPE html>
        <html>
            <head>
                {%metas%}
                <title>{%title%}</title>
                {%favicon%}
                {%css%}
                <style>
                    /* Fix dropdown text color */
                    .Select-value-label, .Select-menu-outer {
                        color: black !important;
                    }
                    /* Fix dropdown menu background */
                    .VirtualizedSelectOption {
                        background-color: white !important;
                    }
                    /* Fix dropdown arrow color */
                    .Select-arrow {
                        border-color: #ccc transparent transparent !important;
                    }
                    /* Status indicator styles */
                    .status-indicator {
                        display: inline-block;
                        width: 15px;
                        height: 15px;
                        border-radius: 50%;
                        margin-right: 8px;
                        vertical-align: middle;
                    }
                    .status-connected {
                        background-color: #28a745;
                        box-shadow: 0 0 10px #28a745;
                    }
                    .status-disconnected {
                        background-color: #dc3545;
                        box-shadow: 0 0 5px #dc3545;
                    }
                </style>
            </head>
            <body>
                {%app_entry%}
                <footer>
                    {%config%}
                    {%scripts%}
                    {%renderer%}
                </footer>
            </body>
        </html>
        '''
        # Set up server
        self.server = self.app.server
        # Define layout and callbacks
        self._create_layout()
        self._register_callbacks()

    def _create_connection_indicator(self, connected):
        """
        Creates a connection status indicator (green for connected, red for disconnected).
        
        Args:
            connected (bool): Whether the connection is active
            
        Returns:
            html.Div: Connection status indicator component
        """
        status_class = "status-connected" if connected else "status-disconnected"
        status_text = "Connected" if connected else "Disconnected"
        
        return html.Div([
            html.Span(className=f"status-indicator {status_class}"),
            html.Span(status_text, className="ms-1", 
                    style={"color": "#28a745" if connected else "#dc3545"})
        ], id="connection-status")

    def _create_connection_controls(self, interface):
        """
        Create the connection toggle and status indicator for each interface.
        
        Args:
            interface (str): The interface to create controls for
            
        Returns:
            dbc.Card: Card component with connection controls
        """
        host_value = {
            "chaser": "192.168.1.110",
            "target": "192.168.1.111",
            "obstacle": "192.168.1.112"
        }[interface]
        
        username_value = {
            "chaser": "spot-red",
            "target": "spot-black",
            "obstacle": "spot-blue"
        }[interface]
        
        return dbc.Card([
            dbc.CardHeader("SSH Connection Settings"),
            dbc.CardBody([
                dbc.Row([
                    dbc.Col([
                        html.Label("Jetson Host/IP:", style={"color": "white"}),
                        dbc.Input(id=f"ssh-host-{interface}", type="text", 
                                placeholder=host_value, value=host_value)
                    ], width=6),
                    dbc.Col([
                        html.Label("SSH Port:", style={"color": "white"}),
                        dbc.Input(id=f"ssh-port-{interface}", type="number", placeholder="22", value="22")
                    ], width=6),
                ]),
                dbc.Row([
                    dbc.Col([
                        html.Label("Username:", style={"color": "white"}),
                        dbc.Input(id=f"ssh-username-{interface}", type="text", 
                                placeholder=username_value, value=username_value)
                    ], width=6),
                    dbc.Col([
                        html.Label("Password:", style={"color": "white"}),
                        dbc.Input(id=f"ssh-password-{interface}", type="password", 
                                placeholder="srcl2023", value="srcl2023")
                    ], width=6),
                ]),
                dbc.Row([
                    dbc.Col([
                        html.Label("SSH Key File (optional):", style={"color": "white"}),
                        dbc.Input(id=f"ssh-key-file-{interface}", type="text", placeholder="/path/to/private_key")
                    ], width=12),
                ]),
                dbc.Row([
                    dbc.Col([
                        dbc.Switch(
                            id=f"ssh-connect-switch-{interface}",
                            label="Establish SSH Connection",
                            value=False,
                            className="mt-3"
                        ),
                    ], width=6),
                    dbc.Col([
                        html.Div(
                            self._create_connection_indicator(False),
                            id=f"ssh-status-{interface}",
                            className="mt-3"
                        )
                    ], width=6),
                ]),
            ])
        ], className="mb-4")

    def _create_layout(self):
        """Define the application layout."""
        # Create tabs
        # tabs = dbc.Tabs([
        #     # Initialize Parameters Tab
        #     dbc.Tab([
        #         html.Div([
        #             html.H3("Initialize Parameters"),
        #             html.P("Parameter initialization options will appear here.")
        #         ], className="p-4")
        #     ], label="Initialize Parameters"),
            
            # # Setup & Run Simulations Tab
            # dbc.Tab([
            #     html.Div([
            #         html.H3("Setup & Run Simulations"),
            #         dbc.Button("Run Simulation", id="run-sim-button", color="primary", className="mb-3"),
            #         html.Div(id="simulation-status"),
            #         dbc.Progress(id="simulation-progress", value=0, striped=True, animated=True, 
            #                     style={"display": "none"})
            #     ], className="p-4")
            # ], label="Setup & Run Simulations"),
            
            # # Setup & Run Experiments Tab
            # dbc.Tab([
            #     html.Div([
            #         html.H3("Setup & Run Experiments"),
            #         html.P("Experiment options coming soon.")
            #     ], className="p-4")
            # ], label="Setup & Run Experiments"),
            
            # Hardware Control Tab

        tabs = dbc.Tabs([dbc.Tab([
                html.Div([
                    html.H3("Hardware Control"),
                    
                    # Sub-tabs for different interfaces
                    dbc.Tabs([
                        # CHASER INTERFACE TAB
                        dbc.Tab([
                            # SSH Connection Parameters
                            self._create_connection_controls("chaser"),
                            
                            # GPIO Control and Thruster Check Cards side by side
                            dbc.Row([
                                # GPIO Control Card
                                dbc.Col([
                                    dbc.Card([
                                        dbc.CardHeader("Air Control"),
                                        dbc.CardBody([
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Label("Air Element:", style={"color": "white"}),
                                                    dcc.Dropdown(
                                                        id="gpio-pin-dropdown-chaser",
                                                        options=[
                                                            {"label": f"Thruster 1", "value": 0},
                                                            {"label": f"Thruster 2", "value": 1},
                                                            {"label": f"Thruster 3", "value": 2},
                                                            {"label": f"Thruster 4", "value": 3},
                                                            {"label": f"Thruster 5", "value": 4},
                                                            {"label": f"Thruster 6", "value": 5},
                                                            {"label": f"Thruster 7", "value": 6},
                                                            {"label": f"Thruster 8", "value": 7},
                                                            {"label": f"Pucks", "value": 100}
                                                        ],
                                                        value=100,
                                                        style={"color": "black"}
                                                    )
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    dbc.Button("Turn ON (HIGH)", id="gpio-on-button-chaser", color="success", 
                                                            className="me-2 mt-3", disabled=True),
                                                    dbc.Button("Turn OFF (LOW)", id="gpio-off-button-chaser", color="danger", 
                                                            className="mt-3", disabled=True),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Div(id="gpio-status-chaser", className="mt-3")
                                                ], width=12),
                                            ]),
                                        ])
                                    ], style={"height": "100%"}),
                                ], width=6),
                                
                                # Thruster Check Card
                                dbc.Col([
                                    dbc.Card([
                                        dbc.CardHeader("Thruster Check"),
                                        dbc.CardBody([
                                            dbc.Row([
                                                dbc.Col([
                                                    html.P("This will activate the following pins in sequence:"),
                                                    html.Label("Duration per pin (seconds):", style={"color": "white"}),
                                                    dbc.Input(
                                                        id="thruster-duration-chaser",
                                                        type="number",
                                                        value=0.5,
                                                        min=0.5,
                                                        max=10,
                                                        step=0.5,
                                                        style={"width": "150px"}
                                                    ),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    dbc.Button(
                                                        "Execute Thruster Check", 
                                                        id="thruster-check-button-chaser", 
                                                        color="warning", 
                                                        className="mt-3",
                                                        disabled=True
                                                    ),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Div(id="thruster-status-chaser", className="mt-3"),
                                                    dbc.Progress(
                                                        id="thruster-progress-chaser",
                                                        value=0,
                                                        striped=True,
                                                        animated=True,
                                                        style={"display": "none", "marginTop": "10px"}
                                                    ),
                                                ], width=12),
                                            ]),
                                        ])
                                    ], style={"height": "100%"}),
                                ], width=6),
                            ]),
                        ], label="Chaser Interface"),
                        
                        # TARGET INTERFACE TAB
                        dbc.Tab([
                            # SSH Connection Parameters
                            self._create_connection_controls("target"),
                            
                            # GPIO Control and Thruster Check Cards side by side
                            dbc.Row([
                                # GPIO Control Card
                                dbc.Col([
                                    dbc.Card([
                                        dbc.CardHeader("Air Control"),
                                        dbc.CardBody([
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Label("Element:", style={"color": "white"}),
                                                    dcc.Dropdown(
                                                        id="gpio-pin-dropdown-target",
                                                        options=[
                                                            {"label": f"Thruster 1", "value": 0},
                                                            {"label": f"Thruster 2", "value": 1},
                                                            {"label": f"Thruster 3", "value": 2},
                                                            {"label": f"Thruster 4", "value": 3},
                                                            {"label": f"Thruster 5", "value": 4},
                                                            {"label": f"Thruster 6", "value": 5},
                                                            {"label": f"Thruster 7", "value": 6},
                                                            {"label": f"Thruster 8", "value": 7},
                                                            {"label": f"Pucks", "value": 100}
                                                        ],
                                                        value=100,
                                                        style={"color": "black"}
                                                    )
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    dbc.Button("Turn ON (HIGH)", id="gpio-on-button-target", color="success", 
                                                            className="me-2 mt-3", disabled=True),
                                                    dbc.Button("Turn OFF (LOW)", id="gpio-off-button-target", color="danger", 
                                                            className="mt-3", disabled=True),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Div(id="gpio-status-target", className="mt-3")
                                                ], width=12),
                                            ]),
                                        ])
                                    ], style={"height": "100%"}),
                                ], width=6),
                                
                                # Thruster Check Card
                                dbc.Col([
                                    dbc.Card([
                                        dbc.CardHeader("Thruster Check"),
                                        dbc.CardBody([
                                            dbc.Row([
                                                dbc.Col([
                                                    html.P("This will activate the following pins in sequence:"),
                                                    html.Label("Duration per pin (seconds):", style={"color": "white"}),
                                                    dbc.Input(
                                                        id="thruster-duration-target",
                                                        type="number",
                                                        value=0.5,
                                                        min=0.5,
                                                        max=10,
                                                        step=0.5,
                                                        style={"width": "150px"}
                                                    ),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    dbc.Button(
                                                        "Execute Thruster Check", 
                                                        id="thruster-check-button-target", 
                                                        color="warning", 
                                                        className="mt-3",
                                                        disabled=True
                                                    ),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Div(id="thruster-status-target", className="mt-3"),
                                                    dbc.Progress(
                                                        id="thruster-progress-target",
                                                        value=0,
                                                        striped=True,
                                                        animated=True,
                                                        style={"display": "none", "marginTop": "10px"}
                                                    ),
                                                ], width=12),
                                            ]),
                                        ])
                                    ], style={"height": "100%"}),
                                ], width=6),
                            ]),
                        ], label="Target Interface"),
                        
                        # OBSTACLE INTERFACE TAB
                        dbc.Tab([
                            # SSH Connection Parameters
                            self._create_connection_controls("obstacle"),
                            
                            # GPIO Control and Thruster Check Cards side by side
                            dbc.Row([
                                # GPIO Control Card
                                dbc.Col([
                                    dbc.Card([
                                        dbc.CardHeader("Air Control"),
                                        dbc.CardBody([
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Label("Element:", style={"color": "white"}),
                                                    dcc.Dropdown(
                                                        id="gpio-pin-dropdown-obstacle",
                                                        options=[
                                                            {"label": f"Thruster 1", "value": 0},
                                                            {"label": f"Thruster 2", "value": 1},
                                                            {"label": f"Thruster 3", "value": 2},
                                                            {"label": f"Thruster 4", "value": 3},
                                                            {"label": f"Thruster 5", "value": 4},
                                                            {"label": f"Thruster 6", "value": 5},
                                                            {"label": f"Thruster 7", "value": 6},
                                                            {"label": f"Thruster 8", "value": 7},
                                                            {"label": f"Pucks", "value": 100}
                                                        ],
                                                        value=100,
                                                        style={"color": "black"}
                                                    )
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    dbc.Button("Turn ON (HIGH)", id="gpio-on-button-obstacle", color="success", 
                                                            className="me-2 mt-3", disabled=True),
                                                    dbc.Button("Turn OFF (LOW)", id="gpio-off-button-obstacle", color="danger", 
                                                            className="mt-3", disabled=True),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Div(id="gpio-status-obstacle", className="mt-3")
                                                ], width=12),
                                            ]),
                                        ])
                                    ], style={"height": "100%"}),
                                ], width=6),
                                
                                # Thruster Check Card
                                dbc.Col([
                                    dbc.Card([
                                        dbc.CardHeader("Thruster Check"),
                                        dbc.CardBody([
                                            dbc.Row([
                                                dbc.Col([
                                                    html.P("This will activate the following pins in sequence:"),
                                                    html.Label("Duration per pin (seconds):", style={"color": "white"}),
                                                    dbc.Input(
                                                        id="thruster-duration-obstacle",
                                                        type="number",
                                                        value=0.5,
                                                        min=0.5,
                                                        max=10,
                                                        step=0.5,
                                                        style={"width": "150px"}
                                                    ),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    dbc.Button(
                                                        "Execute Thruster Check", 
                                                        id="thruster-check-button-obstacle", 
                                                        color="warning", 
                                                        className="mt-3",
                                                        disabled=True
                                                    ),
                                                ], width=12),
                                            ]),
                                            dbc.Row([
                                                dbc.Col([
                                                    html.Div(id="thruster-status-obstacle", className="mt-3"),
                                                    dbc.Progress(
                                                        id="thruster-progress-obstacle",
                                                        value=0,
                                                        striped=True,
                                                        animated=True,
                                                        style={"display": "none", "marginTop": "10px"}
                                                    ),
                                                ], width=12),
                                            ]),
                                        ])
                                    ], style={"height": "100%"}),
                                ], width=6),
                            ]),
                        ], label="Obstacle Interface"),
                    ], className="mb-4"),
                ], className="p-4")
            ], label="Hardware Control"),
            
            # Data Inspector Tab
            dbc.Tab([
                html.Div([
                    html.H3("Data Inspector"),
                    dbc.Row([
                        dbc.Col([
                            dbc.Button("Load NPY Data", id="load-data-button", color="primary", className="me-2"),
                            dcc.Upload(
                                id='upload-data',
                                children=html.Div(['Drag and Drop or ', html.A('Select Files')]),
                                style={
                                    'width': '100%',
                                    'height': '60px',
                                    'lineHeight': '60px',
                                    'borderWidth': '1px',
                                    'borderStyle': 'dashed',
                                    'borderRadius': '5px',
                                    'textAlign': 'center',
                                    'margin': '10px 0'
                                },
                                multiple=False
                            ),
                            html.Div(id="upload-status")
                        ], width=12),
                    ]),
                    dbc.Row([
                        dbc.Col([
                            html.Label("X:", style={"color": "white"}),
                            dcc.Dropdown(
                                id="x-dropdown", 
                                options=[], 
                                value=None,
                                style={"color": "black"}
                            )
                        ], width=3),
                        dbc.Col([
                            html.Label("Y:", style={"color": "white"}),
                            dcc.Dropdown(
                                id="y-dropdown", 
                                options=[], 
                                value=None,
                                style={"color": "black"}
                            )
                        ], width=3),
                        dbc.Col([
                            dbc.Button("Export to PDF", id="export-pdf-button", color="secondary", 
                                    className="mt-4", disabled=True)
                        ], width=3),
                        dbc.Col([
                            html.Div(id="export-status")
                        ], width=3)
                    ], className="mb-4"),
                    dbc.Card([
                        dbc.CardBody([
                            dcc.Graph(id="data-plot", style={"height": "60vh"}, figure=self.data_visualizer.create_empty_plot())
                        ])
                    ])
                ], className="p-4")
            ], label="Data Inspector"),
            # Animations Tab
            dbc.Tab([
                html.Div([
                    html.H3("Spacecraft Animations", className="mb-4"),
                    dbc.Row([
                        dbc.Col([
                            dcc.Upload(
                                id='upload-animation-data',
                                children=html.Div(['Drag and Drop or ', html.A('Select Animation File')]),
                                style={
                                    'width': '100%',
                                    'height': '60px',
                                    'lineHeight': '60px',
                                    'borderWidth': '1px',
                                    'borderStyle': 'dashed',
                                    'borderRadius': '5px',
                                    'textAlign': 'center',
                                    'margin': '0 0 20px 0'  # Better bottom margin
                                },
                                multiple=False
                            ),
                            html.Div(id="animation-upload-status", className="mb-3")
                        ], width=12),
                    ]),
                    # Add responsive container with fixed aspect ratio
                    html.Div([
                        dbc.Card([
                            dbc.CardBody([
                                # Use a div with fixed aspect ratio to maintain plot proportions
                                html.Div([
                                    dcc.Graph(
                                        id="animation-plot",
                                        config={
                                            'displayModeBar': True,
                                            'responsive': True,
                                            'displaylogo': False,
                                        },
                                        style={
                                            "height": "100%",
                                            "width": "100%"
                                        }
                                    )
                                ], style={
                                    "position": "relative",
                                    "paddingBottom": "75%",  # 4:3 aspect ratio
                                    "height": "0",
                                    "overflow": "hidden"
                                })
                            ], className="p-0")  # Remove padding inside card body
                        ], className="mb-4 shadow")
                    ], className="d-flex justify-content-center")
                ], className="p-4")
            ], label="Animations"),          
        ], id="tabs")
        
        # Main layout
        self.app.layout = dbc.Container([
            html.H1("SPOT Web Interface", className="my-4 text-center"),
            tabs,
            dcc.Store(id="store-data-keys"),
            # Add these stores for connection status tracking
            dcc.Store(id="connection-status-update-chaser"),
            dcc.Store(id="connection-status-update-target"),
            dcc.Store(id="connection-status-update-obstacle"),
            dcc.Interval(id="sim-progress-interval", interval=500, disabled=True),
            # Check more frequently - every 2 seconds
            dcc.Interval(id="conn-check-chaser", interval=2000, disabled=False),
            dcc.Interval(id="conn-check-target", interval=2000, disabled=False),
            dcc.Interval(id="conn-check-obstacle", interval=2000, disabled=False),
            dcc.Interval(id="thruster-check-interval-chaser", interval=500, disabled=False),
            dcc.Interval(id="thruster-check-interval-target", interval=500, disabled=False),
            dcc.Interval(id="thruster-check-interval-obstacle", interval=500, disabled=False),
            dcc.Download(id="download-pdf")
        ], fluid=True)

    def _register_callbacks(self):
        """Register all callbacks for the application."""
        self._register_ssh_callbacks()
        self._register_hardware_callbacks()
        #self._register_simulation_callbacks()
        self._register_data_viz_callbacks()
        self._register_animation_callbacks()  # Add this line

    def _register_ssh_callbacks(self):
        """Register callbacks for SSH connection management."""
        
        def create_ssh_connection_callback(interface):
            """Create callback function for SSH connection toggle."""
            def handle_connection_toggle(switch_on, host, port, username, password, key_file):
                # Initialize connection status
                if switch_on:
                    # Validate inputs
                    if not host or not username:
                        return False, self._create_connection_indicator(False), True, True, True
                    
                    # Test connection
                    success = self.ssh_manager.start_monitoring(interface, host, port, username, password, key_file)
                    
                    if success:
                        return switch_on, self._create_connection_indicator(True), False, False, False
                    else:
                        return False, self._create_connection_indicator(False), True, True, True
                else:
                    # Stop background checker
                    self.ssh_manager.stop_monitoring(interface)
                    
                    return switch_on, self._create_connection_indicator(False), True, True, True
            
            return handle_connection_toggle
        
        def create_connection_monitor_callback(interface):
            """Create callback function for connection status monitoring."""
            def update_connection_status(n_intervals):
                # Only check if we're supposed to be monitoring this connection
                if not self.ssh_manager.ssh_connections[interface]["checking"]:
                    return dash.no_update
                
                # Get connection parameters from our stored data
                connection_info = self.ssh_manager.ssh_connections[interface]
                host = connection_info["host"]
                port = connection_info["port"]
                username = connection_info["username"]
                password = connection_info["password"]
                key_file = connection_info["key_file"]
                
                # If we don't have the necessary parameters, don't check
                if not host or not username:
                    return dash.no_update
                
                # Test if the connection is still active
                is_connected = self.ssh_manager.test_ssh_connection(host, port, username, password, key_file)
                
                # Update our connection status
                was_connected = connection_info["connected"]
                connection_info["connected"] = is_connected
                connection_info["last_check"] = time.time()
                
                # If the connection status changed from connected to disconnected, update the UI
                if was_connected and not is_connected:
                    # Connection was lost
                    connection_info["checking"] = False  # Stop checking since we're now disconnected
                    print(f"Connection to {interface} was lost!")
                    
                    # This will trigger a UI update through the connection_status_update Store
                    return f"{time.time()}-disconnected"
                
                # Always return current status - this helps with periodic UI refreshes
                return f"{time.time()}-{'connected' if is_connected else 'disconnected'}"
            
            return update_connection_status
        
        def handle_connection_status_update(status_string):
            """Handle connection status updates."""
            if not status_string:
                return dash.no_update, dash.no_update, dash.no_update, dash.no_update, dash.no_update
            
            # Parse the status string
            try:
                timestamp, status = status_string.split('-')
                is_connected = (status == "connected")
                
                # Update UI based on connection status: 
                # - Status indicator
                # - Thruster button disabled state
                # - GPIO ON button disabled state
                # - GPIO OFF button disabled state
                # - Connection switch value
                return (self._create_connection_indicator(is_connected), 
                        not is_connected,  # Thruster button
                        not is_connected,  # GPIO ON button
                        not is_connected,  # GPIO OFF button
                        is_connected)      # Connection switch
            except:
                return dash.no_update, dash.no_update, dash.no_update, dash.no_update, dash.no_update
        
        # Register callbacks for each interface
        for interface in ["chaser", "target", "obstacle"]:
            # SSH connection toggle callback
            self.app.callback(
                [Output(f"ssh-connect-switch-{interface}", "value"),
                Output(f"ssh-status-{interface}", "children"),
                Output(f"thruster-check-button-{interface}", "disabled"),
                Output(f"gpio-on-button-{interface}", "disabled"),
                Output(f"gpio-off-button-{interface}", "disabled")],
                [Input(f"ssh-connect-switch-{interface}", "value")],
                [State(f"ssh-host-{interface}", "value"),
                State(f"ssh-port-{interface}", "value"),
                State(f"ssh-username-{interface}", "value"),
                State(f"ssh-password-{interface}", "value"),
                State(f"ssh-key-file-{interface}", "value")],
                prevent_initial_call=True
            )(create_ssh_connection_callback(interface))
            
            # Connection monitor callback
            self.app.callback(
                Output(f"connection-status-update-{interface}", "data"),
                [Input(f"conn-check-{interface}", "n_intervals")],
                prevent_initial_call=True
            )(create_connection_monitor_callback(interface))
            
            # Connection status update callback
            self.app.callback(
                [Output(f"ssh-status-{interface}", "children", allow_duplicate=True),
                Output(f"thruster-check-button-{interface}", "disabled", allow_duplicate=True),
                Output(f"gpio-on-button-{interface}", "disabled", allow_duplicate=True),
                Output(f"gpio-off-button-{interface}", "disabled", allow_duplicate=True),
                Output(f"ssh-connect-switch-{interface}", "value", allow_duplicate=True)],
                [Input(f"connection-status-update-{interface}", "data")],
                prevent_initial_call=True
            )(handle_connection_status_update)

    def _register_hardware_callbacks(self):
        """Register callbacks for hardware control."""
        
        def create_gpio_function(interface):
            """Create callback function for GPIO control."""
            def handle_gpio_control(on_clicks, off_clicks, pin, host, port, username, password, key_file):
                # Check if the connection is active
                if not self.ssh_manager.is_connected(interface):
                    return html.Div("Error: No active SSH connection", className="text-danger")
                
                # Determine which button was clicked
                button_id = ctx.triggered_id
                
                if not pin or not host or not username:
                    return html.Div("Error: Please provide Pin, Host, and Username", className="text-danger")
                
                try:
                    # Convert values
                    pin = int(pin)
                    port = int(port) if port else 22
                    
                    thrusters = [0,0,0,0,0,0,0,0]
                    if button_id == f"gpio-on-button-{interface}":
                        if pin >= 0 and pin < 8:
                            thrusters[pin] = 100
                        elif pin == 100:
                            self.hardware_controller.air_bearing = 1
                        action = "ON"

                    elif button_id == f"gpio-off-button-{interface}":
                        if pin >= 0 and pin < 8:
                            thrusters[pin] = 0
                        elif pin == 100:
                            self.hardware_controller.air_bearing = 0                        
                        action = "OFF"    
                    else:
                        return html.Div("No action taken", className="text-warning")
                    
                    # Empty password should be None
                    password = password if password else None
                    key_file = key_file if key_file else None
                    
                    # Check if at least one authentication method is provided
                    if not password and not key_file:
                        return html.Div("Error: Please provide either Password or SSH Key File", className="text-danger")
                    
                    # Call the SSH function
                    success = self.hardware_controller.set_thruster_airbrg(thrusters, self.hardware_controller.air_bearing, host, username, password, key_file, port)
                    
                    if success:
                        if pin >= 0 and pin <= 8:
                            return html.Div(f"Successfully set Thruster {pin+1} to {action}", className="text-success")
                        elif pin == 100:
                            return html.Div(f"Successfully set Pucks to {action}", className="text-success")
                    else:
                        if pin >= 0 and pin <= 8:
                            return html.Div(f"Failed to set Thruster {pin+1} to {action}. Check the console for details.", className="text-danger")
                        elif pin == 100:
                            return html.Div(f"Failed to set Pucks to {action}. Check the console for details.", className="text-danger")

                except Exception as e:
                    return html.Div(f"Error: {str(e)}", className="text-danger")
            
            return handle_gpio_control
        
        def create_thruster_check_function(interface):
            """Create callback function for thruster check."""
            def handle_thruster_check(n_clicks, n_intervals, duration, host, port, username, password, key_file):
                triggered_id = ctx.triggered_id
                
                # Check if connection is active before continuing
                if not self.ssh_manager.is_connected(interface):
                    if triggered_id == f"thruster-check-button-{interface}":
                        return html.Div("Error: No active SSH connection", className="text-danger"), {"display": "none"}, 0, True
                    else:
                        return dash.no_update, dash.no_update, dash.no_update, dash.no_update
                
                # Button was clicked to start the check
                if triggered_id == f"thruster-check-button-{interface}" and n_clicks:
                    if not host or not username:
                        return html.Div("Error: Please provide Host and Username", className="text-danger"), {"display": "none"}, 0, False
                    
                    try:
                        # Convert values
                        duration = float(duration) if duration else 2.0
                        port = int(port) if port else 22
                        
                        # Empty password should be None
                        password = password if password else None
                        key_file = key_file if key_file else None
                        
                        # Check if at least one authentication method is provided
                        if not password and not key_file:
                            return html.Div("Error: Please provide either Password or SSH Key File", className="text-danger"), {"display": "none"}, 0, False
                        
                        # Start the thruster check
                        success = self.hardware_controller.start_thruster_check(
                            interface, duration, host, port, username, password, key_file
                        )
                        
                        if not success:
                            return html.Div("Error starting thruster check", className="text-danger"), {"display": "none"}, 0, False
                        
                        # Update UI
                        progress_style = {"display": "block", "marginTop": "10px"}
                        status_message = html.Div([
                            html.Span("Starting thruster check... ", className="text-warning"),
                            html.Span("This may take a few moments.", className="text-muted")
                        ])
                        
                        return status_message, progress_style, 0, True
                    
                    except Exception as e:
                        return html.Div(f"Error: {str(e)}", className="text-danger"), {"display": "none"}, 0, False
                
                # Interval tick - update progress
                elif triggered_id == f"thruster-check-interval-{interface}":
                    status = self.hardware_controller.thruster_check_status[interface]
                    
                    if not status["running"]:
                        # Check is complete
                        if status["completed"]:
                            total_pins = len(status["thrusters"])
                            failed_pins = status["failed_pins"]
                            
                            if not failed_pins:
                                status_message = html.Div([
                                    html.Div(f"Thruster check completed successfully!", className="text-success"),
                                    html.Div(f"Activated {total_pins} thrusters for {status['duration']} seconds each.", 
                                            className="text-muted mt-2"),
                                    html.Div(f"Total duration: {status['duration'] * total_pins:.1f} seconds", 
                                            className="text-muted")
                                ])
                            else:
                                status_message = html.Div([
                                    html.Div(f"Thruster check completed with errors!", className="text-warning"),
                                    html.Div(f"Failed pins: {', '.join(str(p) for p in failed_pins)}", 
                                            className="text-danger mt-2"),
                                    html.Div(f"Successfully activated {total_pins - len(failed_pins)} out of {total_pins} pins.", 
                                            className="text-muted")
                                ])
                            
                            return status_message, {"display": "none"}, 100, False
                        else:
                            # Just finished initializing
                            return dash.no_update, dash.no_update, dash.no_update, dash.no_update
                    else:
                        # Still running - update progress
                        pins = status["thrusters"]
                        current_index = status["current_thruster"]
                        progress = 0
                        
                        if current_index >= 0:
                            progress = int((current_index + 1) / len(pins) * 100)
                        
                        status_message = html.Div([
                            html.Span(f"Running thruster check... ", className="text-warning"),
                            html.Span(f"Current thruster: {pins[current_index] if current_index < len(pins) and current_index >= 0 else 'Initializing'}", 
                                    className="text-info")
                        ])
                        
                        return status_message, {"display": "block", "marginTop": "10px"}, progress, True
                
                return dash.no_update, dash.no_update, dash.no_update, dash.no_update
            
            return handle_thruster_check
        
        # Register callbacks for each interface
        for interface in ["chaser", "target", "obstacle"]:
            # GPIO control callback
            self.app.callback(
                Output(f"gpio-status-{interface}", "children"),
                [Input(f"gpio-on-button-{interface}", "n_clicks"),
                Input(f"gpio-off-button-{interface}", "n_clicks")],
                [State(f"gpio-pin-dropdown-{interface}", "value"),
                State(f"ssh-host-{interface}", "value"),
                State(f"ssh-port-{interface}", "value"),
                State(f"ssh-username-{interface}", "value"),
                State(f"ssh-password-{interface}", "value"),
                State(f"ssh-key-file-{interface}", "value")],
                prevent_initial_call=True
            )(create_gpio_function(interface))
            
            # Thruster check callback
            self.app.callback(
                [Output(f"thruster-status-{interface}", "children"),
                Output(f"thruster-progress-{interface}", "style"),
                Output(f"thruster-progress-{interface}", "value"),
                Output(f"thruster-check-button-{interface}", "disabled", allow_duplicate=True)],
                [Input(f"thruster-check-button-{interface}", "n_clicks"),
                Input(f"thruster-check-interval-{interface}", "n_intervals")],
                [State(f"thruster-duration-{interface}", "value"),
                State(f"ssh-host-{interface}", "value"),
                State(f"ssh-port-{interface}", "value"),
                State(f"ssh-username-{interface}", "value"),
                State(f"ssh-password-{interface}", "value"),
                State(f"ssh-key-file-{interface}", "value")],
                prevent_initial_call=True
            )(create_thruster_check_function(interface))

    # def _register_simulation_callbacks(self):
    #     """Register callbacks for simulation management."""
        
    #     @self.app.callback(
    #         [Output("simulation-status", "children"),
    #         Output("simulation-progress", "style"),
    #         Output("run-sim-button", "disabled"),
    #         Output("sim-progress-interval", "disabled")],
    #         [Input("run-sim-button", "n_clicks"),
    #         Input("sim-progress-interval", "n_intervals")],
    #         prevent_initial_call=True
    #     )
    #     def handle_simulation(n_clicks, n_intervals):
    #         triggered_id = ctx.triggered_id
            
    #         if triggered_id == "run-sim-button" and n_clicks:
    #             # Start simulation in a separate thread
    #             threading.Thread(
    #                 target=self.simulation_manager.run_simulation, 
    #                 daemon=True
    #             ).start()
    #             return "Simulation running...", {"display": "block"}, True, False
            
    #         if triggered_id == "sim-progress-interval":
    #             if self.simulation_manager.is_simulation_running():
    #                 return "Simulation running...", {"display": "block"}, True, False
    #             else:
    #                 return "Simulation completed!", {"display": "none"}, False, True
            
    #         return dash.no_update, dash.no_update, dash.no_update, dash.no_update

    def _register_data_viz_callbacks(self):
        """Register callbacks for data visualization."""
        
        @self.app.callback(
            [Output("upload-status", "children"),
            Output("x-dropdown", "options"),
            Output("y-dropdown", "options"),
            Output("store-data-keys", "data")],
            [Input("upload-data", "contents")],
            [State("upload-data", "filename")]
        )
        def update_data_options(contents, filename):
            if contents is None:
                return "No file loaded", [], [], None
                
            success, message, keys = self.data_visualizer.load_data_from_contents(contents, filename)
            
            if not success:
                return message, [], [], None
            
            options = [{"label": key, "value": key} for key in keys]
            return message, options, options, keys
        
        @self.app.callback(
            [Output("data-plot", "figure"),
            Output("export-pdf-button", "disabled")],
            [Input("x-dropdown", "value"),
            Input("y-dropdown", "value")],
            [State("store-data-keys", "data")]
        )
        def update_plot(x_key, y_key, keys):
            if not x_key or not y_key:
                return self.data_visualizer.create_empty_plot(), True
            
            figure = self.data_visualizer.create_plot(x_key, y_key)
            return figure, False
        
        @self.app.callback(
            Output("download-pdf", "data"),
            Input("export-pdf-button", "n_clicks"),
            [State("x-dropdown", "value"), State("y-dropdown", "value")],
            prevent_initial_call=True
        )
        def export_plot_to_pdf(n_clicks, x_key, y_key):
            if n_clicks is None or not x_key or not y_key:
                return dash.no_update
            
            pdf_bytes = self.data_visualizer.export_to_pdf(x_key, y_key)
            
            if pdf_bytes:
                return dcc.send_bytes(pdf_bytes, f"{y_key}_vs_{x_key}.pdf")
            else:
                return dash.no_update

    def _register_animation_callbacks(self):
        """Register callbacks for animation functionality."""
        
        @self.app.callback(
            [Output("animation-upload-status", "children"),
            Output("animation-plot", "figure")],
            [Input("upload-animation-data", "contents")],
            [State("upload-animation-data", "filename")]
        )
        def update_animation(contents, filename):
            # Create an empty figure for error cases
            def create_empty_fig(message):
                empty_fig = go.Figure()
                empty_fig.update_layout(
                    template="plotly_dark",
                    plot_bgcolor='#333333',
                    paper_bgcolor='#333333',
                    margin=dict(l=40, r=40, t=40, b=40),
                    autosize=True,
                    height=500
                )
                empty_fig.add_annotation(
                    text=message,
                    xref="paper", yref="paper",
                    x=0.5, y=0.5,
                    showarrow=False,
                    font=dict(color="white", size=16)
                )
                return empty_fig
            
            # Handle the case where no file has been uploaded yet
            if contents is None or filename is None:
                return "No file loaded", create_empty_fig("Upload a .npy file with spacecraft data")
            
            # Try to load the animation data
            success, message = self.animation_manager.load_animation_data(contents, filename)
            
            if not success:
                return message, create_empty_fig(message)
            
            # Try to create the animation
            try:
                fig = self.animation_manager.create_animation()
                if fig is None:
                    return "Error: Could not create animation", create_empty_fig("Animation creation failed")
                return message, fig
            except Exception as e:
                error_msg = f"Error creating animation: {str(e)}"
                return error_msg, create_empty_fig(error_msg)

    def run(self, debug=True, host='0.0.0.0'):
        """
        Run the application server.
        
        Args:
            debug (bool): Whether to run in debug mode
            host (str): Host to run the server on
        """
        self.app.run(debug=debug, host=host)

# Create and run the application
app = ProxiPyApp()
app.run(debug=True, host='0.0.0.0')
