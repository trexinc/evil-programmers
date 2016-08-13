import sys
import os
import tempfile

class Logger(object):
    def __init__(self):
        self.terminal = sys.stdout
        self.log = open(os.path.join(tempfile.gettempdir(), "pygin.log"), "a")

    def write(self, message):
        self.terminal.write(message)
        self.log.write(message)
        self.flush()

    def flush(self):
        self.log.flush()

Log = Logger()
sys.stdout = Log
sys.stderr = Log
