import ntplib
import time
from datetime import datetime, timezone
import socket
import struct


# Alternative: More concise version using total_seconds()
def sync_with_ntp_alt(ntp_server='pool.ntp.org'):
    """Alternative implementation using timedelta.total_seconds() with failsafe"""
    try:
        client = ntplib.NTPClient()
        response = client.request(ntp_server, version=3)
        
        offset = response.offset
        ntp_time = time.time() + offset
        ntp_datetime = datetime.fromtimestamp(ntp_time, timezone.utc)
        
        # Create midnight of the same day
        midnight = ntp_datetime.replace(hour=0, minute=0, second=0, microsecond=0)
        
        # Calculate seconds since midnight
        seconds_of_day = (ntp_datetime - midnight).total_seconds()
        
    except Exception:
        # Failsafe: use local time if NTP fails
        local_dt = datetime.now(timezone.utc)
        midnight = local_dt.replace(hour=0, minute=0, second=0, microsecond=0)
        seconds_of_day = (local_dt - midnight).total_seconds()
        ntp_time = time.time()
        offset = 0.0

    # Send seconds_of_day as double (8 bytes) over UDP port 42143 (broadcast), non-blocking
    udp_port = 42143
    udp_ip = '<broadcast>'
    message = struct.pack('d', seconds_of_day)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.setblocking(False)
    try:
        sock.sendto(message, (udp_ip, udp_port))
    except Exception:
        pass  # Ignore send errors in non-blocking mode
    sock.close()
    
    return ntp_time, offset, seconds_of_day


while True:
    ntp_time, offset, seconds_of_day = sync_with_ntp_alt()
