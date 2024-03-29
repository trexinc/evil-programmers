﻿"""
Plugin loader
"""
"""
_loader.py
"""
"""
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

import sys
import os.path
import importlib
from importlib.util import spec_from_file_location, module_from_spec


def _reload_plugin(name: str, spec):
	# Python isn't smart enough to find a spec for a module that wasn't loaded from sys.path
	class FarPluginSpecImporter:
		@classmethod
		def find_spec(cls, fullname, path=None, target=None):
			return spec if fullname == name else None

	sys.meta_path.append(FarPluginSpecImporter)
	try:
		return importlib.reload(sys.modules[name])
	finally:
		sys.meta_path.remove(FarPluginSpecImporter)

def _load_plugin_impl(name: str, path: str):
	spec = spec_from_file_location(name, path)

	if name in sys.modules.keys():
		return _reload_plugin(name, spec)

	module = module_from_spec(spec)
	sys.modules[name] = module
	spec.loader.exec_module(module)
	return module

def _load_plugin(name: str, path: str):
	package_path = os.path.dirname(path)
	package_init = os.path.join(package_path, "__init__.py")
	if os.path.exists(package_init):
		package_name = os.path.basename(package_path)
		_load_plugin_impl(package_name, package_init)
		name = package_name + "." + name

	return _load_plugin_impl(name, path)
