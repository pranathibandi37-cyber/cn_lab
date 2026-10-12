# EX.NO 8 - TCP and UDP with Traffic Control Mechanisms (Cisco Packet Tracer)
# Same code on both PCs. Only change ROLE:
#   PC0 (192.168.1.1) -> ROLE = "server"   (run FIRST)
#   PC1 (192.168.1.2) -> ROLE = "client"
# TCP: token bucket (shaping) + sliding window (flow control) + slow start/AIMD (congestion control)
# UDP: leaky bucket (shaping) + Stop-and-Wait ARQ (reliability over a lossy network)
from tcp import *
from udp import *
from time import *

ROLE = "server"
SERVER_IP = "192.168.1.1"
TCP_PORT = 5000
UDP_PORT = 6000
UDP_CLIENT_PORT = 6001
MSG = "Hello this is 24BCE2079"
CHUNK = 4

RWND = 3                   # TCP receiver window (flow control)
TCP_DROP = [5]             # TCP segment 5 lost once (congestion)
UDP_DROP_DATA = [1, 2, 4, 4]
UDP_DROP_ACK = [4]

RATE = 5                   # token bucket: tokens per second
CAP = 3                    # token bucket size
INTERVAL = 0.5             # leaky bucket: 1 packet every 0.5 s
TIMEOUT = 2
MAX_RETRIES = 10


def split_msg():
    chunks = []
    for i in range(0, len(MSG), CHUNK):
        chunks.append(MSG[i:i + CHUNK])
    return chunks


# ======================= SERVER (PC0) =======================
tcp_server = TCPServer()
udp_sock = UDPSocket()
udp_st = {"expected": 0, "msg": "", "printed": False}


def onTCPNewClient(client):
    st = {"expected": 0, "msg": "", "buf": ""}

    def onChange(type):
        print("[TCP] client " + client.remoteIP() + " state: " + str(type))

    def handle(line):
        if line == "FIN":
            print("")
            print("[TCP] Full message received: " + st["msg"])
            client.send("DONE|" + st["msg"] + "\n")
            return
        parts = line.split("|", 2)
        seq = int(parts[1])
        if seq in TCP_DROP:
            TCP_DROP.remove(seq)
            reply = "NAK"
            print("  seq=" + str(seq) + " LOST (simulated congestion) -> NAK")
        elif seq == st["expected"]:
            st["msg"] = st["msg"] + parts[2]
            st["expected"] = st["expected"] + 1
            reply = "ACK"
            print("  seq=" + str(seq) + " accepted '" + parts[2] + "' -> ACK (rwnd=" + str(RWND) + ")")
        else:
            reply = "NAK"
            print("  seq=" + str(seq) + " out of order, discarded -> NAK")
        client.send(reply + "|" + str(seq) + "|" + str(RWND) + "\n")

    def onReceive(data):
        st["buf"] = st["buf"] + data
        while "\n" in st["buf"]:
            parts = st["buf"].split("\n", 1)
            st["buf"] = parts[1]
            handle(parts[0])

    client.onConnectionChange(onChange)
    client.onReceive(onReceive)


def onUDPServerReceive(ip, port, data):
    if data == "START":
        udp_st["expected"], udp_st["msg"], udp_st["printed"] = 0, "", False
        print("")
        print("[UDP] new session from " + ip + ":" + str(port))
        udp_sock.send(ip, port, "START_ACK")
        return
    if data == "FIN":
        if not udp_st["printed"]:
            print("")
            print("[UDP] Full message received: " + udp_st["msg"])
            udp_st["printed"] = True
        udp_sock.send(ip, port, "DONE|" + udp_st["msg"])
        return
    parts = data.split("|", 2)
    seq = int(parts[1])
    if seq in UDP_DROP_DATA:
        UDP_DROP_DATA.remove(seq)
        print("  seq=" + str(seq) + " DROPPED (simulated loss)")
        return
    if seq == udp_st["expected"]:
        udp_st["msg"] = udp_st["msg"] + parts[2]
        udp_st["expected"] = udp_st["expected"] + 1
        print("  seq=" + str(seq) + " received '" + parts[2] + "'")
    else:
        print("  seq=" + str(seq) + " duplicate (ACK was lost), re-ACK")
    if seq in UDP_DROP_ACK:
        UDP_DROP_ACK.remove(seq)
        print("  ACK " + str(seq) + " DROPPED (simulated loss)")
        return
    udp_sock.send(ip, port, "ACK|" + str(seq))


def run_server():
    tcp_server.onNewClient(onTCPNewClient)
    print("[TCP] server listening on port " + str(TCP_PORT) + ": " + str(tcp_server.listen(TCP_PORT)))
    udp_sock.onReceive(onUDPServerReceive)
    print("[UDP] server listening on port " + str(UDP_PORT) + ": " + str(udp_sock.begin(UDP_PORT)))
    while True:
        sleep(3600)


# ======================= CLIENT (PC1) =======================
tcp_client = TCPClient()
tcp_replies = []
udp_replies = []
cst = {"buf": "", "done": None, "tokens": CAP, "sent": 0, "retx": 0}


def onTCPClientReceive(data):
    cst["buf"] = cst["buf"] + data
    while "\n" in cst["buf"]:
        parts = cst["buf"].split("\n", 1)
        cst["buf"] = parts[1]
        if parts[0].startswith("DONE|"):
            cst["done"] = parts[0].split("|", 1)[1]
        else:
            tcp_replies.append(parts[0])


def onUDPClientReceive(ip, port, data):
    udp_replies.append(data)


def tick():
    sleep(0.1)
    cst["tokens"] = min(CAP, cst["tokens"] + RATE * 0.1)


def consume_token():
    while cst["tokens"] < 1:
        print("    [token bucket] empty, waiting...")
        tick()
        tick()
    cst["tokens"] = cst["tokens"] - 1


def show_result(received):
    print("")
    print("Sent     : " + MSG)
    print("Received : " + str(received))
    if received == MSG:
        print("RESULT   : SUCCESS - 100% delivered")
    else:
        print("RESULT   : MISMATCH")


def run_tcp_client():
    print("========== TCP ==========")
    tcp_client.onReceive(onTCPClientReceive)
    print("Connecting to " + SERVER_IP + ":" + str(TCP_PORT) + " -> " + str(tcp_client.connect(SERVER_IP, TCP_PORT)))
    sleep(3)   # wait for the 3-way handshake
    chunks = split_msg()
    n = len(chunks)
    print("Message split into " + str(n) + " segments")

    cwnd, ssthresh, rwnd, base, rnd = 1, 8, 4, 0, 0
    while base < n:
        rnd = rnd + 1
        win = min(cwnd, rwnd)
        end = min(base + win, n)
        note = ""
        if rwnd < cwnd:
            note = "  <- limited by receiver (flow control)"
        print("")
        print("Round " + str(rnd) + ": cwnd=" + str(cwnd) + " rwnd=" + str(rwnd) +
              " ssthresh=" + str(ssthresh) + " -> window=" + str(win) + note)

        for seq in range(base, end):
            consume_token()
            tcp_client.send("SEQ|" + str(seq) + "|" + chunks[seq] + "\n")
            print("  sent seq=" + str(seq) + " data='" + chunks[seq] + "'")

        t = 0
        while len(tcp_replies) < end - base and t < 100:
            tick()
            t = t + 1

        acked, lost = base, -1
        for k in range(end - base):
            if not tcp_replies:
                break
            parts = tcp_replies.pop(0).split("|")
            seq = int(parts[1])
            rwnd = int(parts[2])
            if parts[0] == "ACK" and lost < 0:
                acked = seq + 1
                print("  ACK " + str(seq))
            elif lost < 0:
                lost = seq
                print("  NAK " + str(seq) + " (packet lost)")
        if acked < end and lost < 0:
            lost = acked   # no reply in time = timeout

        if lost >= 0:
            ssthresh = max(cwnd // 2, 1)
            cwnd = 1
            print("  CONGESTION -> ssthresh=" + str(ssthresh) + ", cwnd=1, resend from seq " +
                  str(acked) + " (Go-Back-N)")
            base = acked
        else:
            base = end
            if cwnd < ssthresh:
                cwnd = cwnd * 2
                print("  slow start -> cwnd=" + str(cwnd))
            else:
                cwnd = cwnd + 1
                print("  congestion avoidance -> cwnd=" + str(cwnd))

    tcp_client.send("FIN\n")
    t = 0
    while cst["done"] is None and t < 100:
        tick()
        t = t + 1
    show_result(cst["done"])
    tcp_client.close()


def udp_send_reliable(sock, payload, want, prefix):
    for attempt in range(1, MAX_RETRIES + 1):
        sleep(INTERVAL)                       # leaky bucket: constant rate
        sock.send(SERVER_IP, UDP_PORT, payload)
        cst["sent"] = cst["sent"] + 1
        if attempt > 1:
            cst["retx"] = cst["retx"] + 1
        t = 0
        while t < TIMEOUT * 10:
            while udp_replies:
                r = udp_replies.pop(0)
                if r == want or (prefix and r.startswith(want)):
                    return r
            sleep(0.1)
            t = t + 1
        print("    timeout (attempt " + str(attempt) + ") -> retransmit")
    print("No reply from server. Is the server running?")
    return None


def run_udp_client():
    print("")
    print("========== UDP ==========")
    sock = UDPSocket()
    sock.onReceive(onUDPClientReceive)
    print("UDP client on port " + str(UDP_CLIENT_PORT) + ": " + str(sock.begin(UDP_CLIENT_PORT)))
    chunks = split_msg()
    print("Sending " + str(len(chunks)) + " packets, leaky bucket = 1 packet / " + str(INTERVAL) + " s")

    if udp_send_reliable(sock, "START", "START_ACK", False) is None:
        return
    for seq in range(len(chunks)):
        print("  send seq=" + str(seq) + " data='" + chunks[seq] + "'")
        if udp_send_reliable(sock, "SEQ|" + str(seq) + "|" + chunks[seq], "ACK|" + str(seq), False) is None:
            return
        print("  ACK " + str(seq))
    reply = udp_send_reliable(sock, "FIN", "DONE|", True)
    received = ""
    if reply:
        received = reply.split("|", 1)[1]
    print("Packets  : " + str(len(chunks)) + " data, " + str(cst["sent"]) +
          " transmissions, " + str(cst["retx"]) + " retransmissions")
    show_result(received)


def main():
    if ROLE == "server":
        run_server()
    else:
        run_tcp_client()
        sleep(2)
        run_udp_client()


if __name__ == "__main__":
    main()
