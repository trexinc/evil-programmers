import sys
from importlib.util import spec_from_file_location, module_from_spec

def load_plugin(name, path):
	spec_file = spec_from_file_location(name, path)
	spec_module = module_from_spec(spec_file)
	spec_file.loader.exec_module(spec_module)
	sys.modules[name] = spec_module
	return spec_module
