#!/usr/bin/env python
import optparse
import os
import pprint
import re
import shlex
import subprocess
import sys
import shutil
import string

# gcc and g++ as defaults matches what GYP's Makefile generator does,
# except on OS X.
CC = os.environ.get('CC', 'cc' if sys.platform == 'darwin' else 'gcc')
CXX = os.environ.get('CXX', 'c++' if sys.platform == 'darwin' else 'g++')

root_dir = os.path.dirname(__file__)
sys.path.insert(0, os.path.join(root_dir, 'tools', 'gyp', 'pylib'))
from gyp.common import GetFlavor

# imports in tools/configure.d
sys.path.insert(1, os.path.join(root_dir, 'tools', 'configure.d'))

sys.path.insert(1, os.path.join(root_dir, 'src', 'client', 'plugins'))
sys.path.insert(1, os.path.join(root_dir, 'src', 'server', 'plugins'))

# parse our options
parser = optparse.OptionParser()

valid_os = ('win', 'mac', 'solaris', 'freebsd', 'openbsd', 'linux', 'android')
valid_arch = ('arm', 'arm64', 'ia32', 'mips', 'mipsel', 'x32', 'x64', 'x86')
valid_arm_float_abi = ('soft', 'softfp', 'hard')
valid_mips_arch = ('loongson', 'r1', 'r2', 'r6', 'rx')
valid_mips_fpu = ('fp32', 'fp64', 'fpxx')
valid_mips_float_abi = ('soft', 'hard')
valid_intl_modes = ('none', 'small-icu', 'full-icu', 'system-icu')

# create option groups
shared_optgroup = optparse.OptionGroup(parser, "Shared libraries",
    "Flags that allows you to control whether you want to build against "
    "built-in dependencies or its shared representations. If necessary, "
    "provide multiple libraries with comma.")
intl_optgroup = optparse.OptionGroup(parser, "Internationalization",
    "Flags that lets you enable i18n features in io.js as well as which "
    "library you want to build against.")

# Options should be in alphabetical order but keep --prefix at the top,
# that's arguably the one people will be looking for most.
parser.add_option('--prefix',
    action='store',
    dest='prefix',
    default='/usr/local',
    help='select the install prefix [default: %default]')

parser.add_option('--debug',
    action='store_true',
    dest='debug',
    help='also build debug build')

parser.add_option('--dest-cpu',
    action='store',
    dest='dest_cpu',
    choices=valid_arch,
    help='CPU architecture to build for ({0})'.format(', '.join(valid_arch)))

parser.add_option('--dest-os',
    action='store',
    dest='dest_os',
    choices=valid_os,
    help='operating system to build for ({0})'.format(', '.join(valid_os)))

parser.add_option('--gdb',
    action='store_true',
    dest='gdb',
    help='add gdb support')

parser.add_option("--fully-static",
    action="store_true",
    dest="fully_static",
    help="Generate an executable without external dynamic libraries. This "
         "will not work on OSX when using default compilation environment")

parser.add_option_group(shared_optgroup)

# TODO document when we've decided on what the tracing API and its options will
# look like
parser.add_option('--systemtap-includes',
    action='store',
    dest='systemtap_includes',
    help=optparse.SUPPRESS_HELP)

parser.add_option('--tag',
    action='store',
    dest='tag',
    help='custom build tag')



parser.add_option_group(intl_optgroup)

parser.add_option('--enable-static',
    action='store_true',
    dest='enable_static',
    help='build as static library')

(options, args) = parser.parse_args()

def warn(msg):
  warn.warned = True
  prefix = '\033[1m\033[93mWARNING\033[0m' if os.isatty(1) else 'WARNING'
  print('%s: %s' % (prefix, msg))

# track if warnings occured
warn.warned = False

def execute(argv, env=os.environ):
  try:
    output = subprocess.check_output(argv, stderr=subprocess.STDOUT, env=env, shell=True)
    return output
  except subprocess.CalledProcessError as e:
    print e.output
  raise e


def execute_stdout(argv, env=os.environ):
  execute(argv, env)

def b(value):
  """Returns the string 'true' if value is truthy, 'false' otherwise."""
  if value:
    return 'true'
  else:
    return 'false'


def pkg_config(pkg):
  pkg_config = os.environ.get('PKG_CONFIG', 'pkg-config')
  args = '--silence-errors'
  retval = ()
  for flag in ['--libs-only-l', '--cflags-only-I', '--libs-only-L']:
    try:
      val = subprocess.check_output([pkg_config, args, flag, pkg])
      # check_output returns bytes
      val = val.encode().strip().rstrip('\n')
    except subprocess.CalledProcessError:
      # most likely missing a .pc-file
      val = None
    except OSError:
      # no pkg-config/pkgconf installed
      return (None, None, None)
    retval += (val,)
  return retval


def try_check_compiler(cc, lang):
  try:
    proc = subprocess.Popen(shlex.split(cc) + ['-E', '-P', '-x', lang, '-'],
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE)
  except OSError:
    return (False, False, '', '')

  proc.stdin.write('__clang__ __GNUC__ __GNUC_MINOR__ __GNUC_PATCHLEVEL__ '
                   '__clang_major__ __clang_minor__ __clang_patchlevel__')

  values = (proc.communicate()[0].split() + ['0'] * 7)[0:7]
  is_clang = values[0] == '1'
  gcc_version = '%s.%s.%s' % tuple(values[1:1+3])
  clang_version = '%s.%s.%s' % tuple(values[4:4+3])

  return (True, is_clang, clang_version, gcc_version)


#
# The version of asm compiler is needed for building openssl asm files.
# See deps/openssl/openssl.gypi for detail.
# Commands and reglar expressions to obtain its version number is taken from
# https://github.com/openssl/openssl/blob/OpenSSL_1_0_2-stable/crypto/sha/asm/sha512-x86_64.pl#L112-L129
#
def get_llvm_version(cc):
  try:
    proc = subprocess.Popen(shlex.split(cc) + ['-v'], stdin=subprocess.PIPE,
                            stderr=subprocess.PIPE, stdout=subprocess.PIPE)
  except OSError:
    print '''diorama-native-modding configure error: No acceptable C compiler found!

        Please make sure you have a C compiler installed on your system and/or
        consider adjusting the CC environment variable if you installed
        it in a non-standard prefix.
        '''
    sys.exit()

  match = re.search(r"(^clang version|based on LLVM) ([3-9]\.[0-9]+)",
                    proc.communicate()[1])

  if match:
    return match.group(2)
  else:
    return 0

# Note: Apple clang self-reports as clang 4.2.0 and gcc 4.2.1.  It passes
# the version check more by accident than anything else but a more rigorous
# check involves checking the build number against a whitelist.  I'm not
# quite prepared to go that far yet.
def check_compiler(o):
  if sys.platform == 'win32':
    return

  ok, is_clang, clang_version, gcc_version = try_check_compiler(CXX, 'c++')
  if not ok:
    warn('failed to autodetect C++ compiler version (CXX=%s)' % CXX)
  elif clang_version < '3.4.0' if is_clang else gcc_version < '4.8.0':
    warn('C++ compiler too old, need g++ 4.8 or clang++ 3.4 (CXX=%s)' % CXX)

  ok, is_clang, clang_version, gcc_version = try_check_compiler(CC, 'c')
  if not ok:
    warn('failed to autodetect C compiler version (CC=%s)' % CC)
  elif not is_clang and gcc_version < '4.2.0':
    # clang 3.2 is a little white lie because any clang version will probably
    # do for the C bits.  However, we might as well encourage people to upgrade
    # to a version that is not completely ancient.
    warn('C compiler too old, need gcc 4.2 or clang 3.2 (CC=%s)' % CC)

  if is_clang:
    o['variables']['llvm_version'] = get_llvm_version(CC)


def cc_macros():
  """Checks predefined macros using the CC command."""

  try:
    p = subprocess.Popen(shlex.split(CC) + ['-dM', '-E', '-'],
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
  except OSError:
    print '''diorama-native-modding configure error: No acceptable C compiler found!

        Please make sure you have a C compiler installed on your system and/or
        consider adjusting the CC environment variable if you installed
        it in a non-standard prefix.
        '''
    sys.exit()

  p.stdin.write('\n')
  out = p.communicate()[0]

  out = str(out).split('\n')

  k = {}
  for line in out:
    lst = shlex.split(line)
    if len(lst) > 2:
      key = lst[1]
      val = lst[2]
      k[key] = val
  return k


def is_arch_armv7():
  """Check for ARMv7 instructions"""
  cc_macros_cache = cc_macros()
  return ('__ARM_ARCH_7__' in cc_macros_cache or
          '__ARM_ARCH_7A__' in cc_macros_cache or
          '__ARM_ARCH_7R__' in cc_macros_cache or
          '__ARM_ARCH_7M__' in cc_macros_cache or
          '__ARM_ARCH_7S__' in cc_macros_cache)


def is_arch_armv6():
  """Check for ARMv6 instructions"""
  cc_macros_cache = cc_macros()
  return ('__ARM_ARCH_6__' in cc_macros_cache or
          '__ARM_ARCH_6M__' in cc_macros_cache)


def is_arm_hard_float_abi():
  """Check for hardfloat or softfloat eabi on ARM"""
  # GCC versions 4.6 and above define __ARM_PCS or __ARM_PCS_VFP to specify
  # the Floating Point ABI used (PCS stands for Procedure Call Standard).
  # We use these as well as a couple of other defines to statically determine
  # what FP ABI used.

  return '__ARM_PCS_VFP' in cc_macros()


def host_arch_cc():
  """Host architecture check using the CC command."""

  k = cc_macros()

  matchup = {
    '__aarch64__' : 'arm64',
    '__arm__'     : 'arm',
    '__i386__'    : 'ia32',
    '__mips__'    : 'mips',
    '__x86_64__'  : 'x64',
  }

  rtn = 'ia32' # default

  for i in matchup:
    if i in k and k[i] != '0':
      rtn = matchup[i]
      break

  return rtn


def host_arch_win():
  """Host architecture check using environ vars (better way to do this?)"""

  observed_arch = os.environ.get('PROCESSOR_ARCHITECTURE', 'x86')
  arch = os.environ.get('PROCESSOR_ARCHITEW6432', observed_arch)

  matchup = {
    'AMD64'  : 'x64',
    'x86'    : 'ia32',
    'arm'    : 'arm',
    'mips'   : 'mips',
  }

  return matchup.get(arch, 'ia32')

def configure_library(lib, output):
  shared_lib = 'shared_' + lib
  output['variables']['diorama_native_modding_' + shared_lib] = b(getattr(options, shared_lib))

  if getattr(options, shared_lib):
    (pkg_libs, pkg_cflags, pkg_libpath) = pkg_config(lib)

    if pkg_cflags:
      output['include_dirs'] += (
          filter(None, map(str.strip, pkg_cflags.split('-I'))))

    # libpath needs to be provided ahead libraries
    if pkg_libpath:
      output['libraries'] += (
          filter(None, map(str.strip, pkg_cflags.split('-L'))))

    default_libs = getattr(options, shared_lib + '_libname')
    default_libs = map('-l{0}'.format, default_libs.split(','))

    if pkg_libs:
      output['libraries'] += pkg_libs.split()
    elif default_libs:
      output['libraries'] += default_libs

def configure_fullystatic(o):
  if options.fully_static:
    o['libraries'] += ['-static']
    if flavor == 'mac':
      print("Generation of static executable will not work on OSX "
            "when using default compilation environment")

def write(filename, data):
  filename = os.path.join(root_dir, filename)
  print 'creating ', filename
  f = open(filename, 'w+')
  f.write(data)

do_not_edit = '# Do not edit. Generated by the configure script.\n'

def glob_to_var(dir_base, dir_sub):
  list = []
  dir_all = os.path.join(dir_base, dir_sub)
  files = os.walk(dir_all)
  for ent in files:
    (path, dirs, files) = ent
    for file in files:
      if file.endswith('.cpp') or file.endswith('.c') or file.endswith('.h'):
        list.append('%s/%s' % (dir_sub, file))
    break
  return list

output = {
  'variables': { 'python': sys.executable,
                  'deps_path': os.path.relpath('deps/') },
  'include_dirs': [],
  'libraries': [],
  'defines': [],
  'cflags': [],
}

host_arch = host_arch_win() if os.name == 'nt' else host_arch_cc()
target_arch = 'x86' #options.dest_cpu or host_arch

# ia32 is preferred by the build tools (GYP) over x86 even if we prefer the latter
# the Makefile resets this to x86 afterward
if target_arch == 'x86':
  target_arch = 'ia32'

output['variables']['host_arch'] = host_arch
output['variables']['target_arch'] = target_arch

# Print a warning when the compiler is too old.
check_compiler(output)

# determine the "flavor" (operating system) we're building for,
# leveraging gyp's GetFlavor function
flavor_params = {}
if (options.dest_os):
  flavor_params['flavor'] = options.dest_os
flavor = GetFlavor(flavor_params)

configure_fullystatic(output)

# variables should be a root level element,
# move everything else to target_defaults
variables = output['variables']
del output['variables']

# make_global_settings should be a root level element too
if 'make_global_settings' in output:
  make_global_settings = output['make_global_settings']
  del output['make_global_settings']
else:
  make_global_settings = False

output = {
  'variables': variables,
  'target_defaults': output,
}
if make_global_settings:
  output['make_global_settings'] = make_global_settings

pprint.pprint(output, indent=2)

write('config.gypi', do_not_edit +
      pprint.pformat(output, indent=2) + '\n')

config = {
  'BUILDTYPE': 'Debug' if options.debug else 'Release',
 # 'USE_XCODE': str(int(options.use_xcode or 0)),
  'PYTHON': sys.executable,
}

if options.prefix:
  config['PREFIX'] = options.prefix

config = '\n'.join(map('='.join, config.iteritems())) + '\n'

write('config.mk',
      '# Do not edit. Generated by the configure script.\n' + config)

gyp_args = [sys.executable, 'tools/gyp_diorama_native_modding.py', '--no-parallel']

#if options.use_xcode:
#  gyp_args += ['-f', 'xcode']
#elif flavor == 'win' and sys.platform != 'msys':
if flavor == 'win' and sys.platform != 'msys':
  gyp_args += ['-f', 'msvs', '-G', 'msvs_version=auto']
else:
  gyp_args += ['-f', 'make-' + flavor]

gyp_args += args

if warn.warned:
  warn('warnings were emitted in the configure phase')

sys.exit(subprocess.call(gyp_args))
