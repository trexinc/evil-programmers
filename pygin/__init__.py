import sys
import os
import tempfile

class Logger(object):
    def __init__(self):
        self.terminal = sys.stdout
        self.logfile = os.path.join(tempfile.gettempdir(), "pygin.log")

    def write(self, message):
        with open(self.logfile, "a") as log:
            log.write(message)
        self.terminal.write(message)
        self.flush()

    def flush(self):
        self.terminal.flush()

Log = Logger()
sys.stdout = Log
sys.stderr = Log
