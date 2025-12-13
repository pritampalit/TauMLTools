## see https://github.com/riga/law/tree/master/examples/htcondor_at_cern

import six
import subprocess
import os
import re
import sys
import shutil

import law
import luigi
from .framework import Task, HTCondorWorkflow
from RunKit.sh_tools import sh_call

class SimpleMergeMix(Task, HTCondorWorkflow, law.LocalWorkflow):
  ## '_' will be converted to '-' for the shell command invocation
  cfg               = luigi.Parameter(description = 'configuration file with the list of input sources')
  input_path        = luigi.Parameter(description = 'Input file with the list of files to read. '
                                                    'The --prefix argument will be placed in front of --input.')
  output_path       = luigi.Parameter(description = 'output directory')
  input_spec        = luigi.Parameter(description = '')
  n_jobs            = luigi.IntParameter(description = 'number of HTCondor jobs to run')
  ## optional arguments (default will be an empty string: don't override ShuffleMergeSpectral.cxx default values)
  prefix            = luigi.Parameter(description = 'Prefix to place before the input file path read from --input.'
                                                    'It can include a remote server to use with xrootd.', default = "")
  compression_algo  = luigi.Parameter(default = '', description = 'ZLIB, LZMA, LZ4')
  compression_level = luigi.Parameter(default = '', description = 'compression level of output file')
  disabled_branches = luigi.Parameter(default = '', description = 'disabled-branches list of branches to disabled in the input tuples')
  max_entries       = luigi.Parameter(default = '', description = 'maximal number of entries in output train+test tuples')
  n_threads         = luigi.Parameter(default = '', description = 'number of threads')
  seed              = luigi.Parameter(default = '', description = 'random seed to initialize the generator used for sampling')

  def create_branch_map(self):
    #if not (self.mode == 'MergeAll' or self.mode == ''):
    #  raise Exception('Only --mode MergeAll is supported by the law tool')
    if os.path.isabs(self.output_path):
      output = self.output_path
    else:
      output = os.path.join(os.environ['ANALYSIS_PATH'], self.output_path)

    tmp_output = os.path.join(output, 'tmp')
    hash_output = os.path.join(output, 'hashes')
    os.makedirs(tmp_output, exist_ok = True)
    os.makedirs(hash_output, exist_ok = True)
    branches = {}
    for i in range(self.n_jobs):
      branches[i] = (output, tmp_output, hash_output)
    return branches

  def output(self):
    output, tmp_output, hash_output = self.branch_data
    file_name = self.getFileName()
    return law.LocalFileTarget(os.path.join(output, file_name))

  def getFileName(self):
    return f'SimpleMergeMix_{self.branch}.root'

  def move(self, src, dest):
    if os.path.exists(dest):
      if os.path.isdir(dest): shutil.rmtree(dest)
      else: os.remove(dest)
    shutil.move(src, dest)

  def run(self):
    output, tmp_output, hash_output = self.branch_data
    file_name   = self.getFileName()
    tmp_file = os.path.join(tmp_output, file_name)

    os.environ["LD_LIBRARY_PATH"] = "/usr/lib64:" + os.environ.get("LD_LIBRARY_PATH", "")
    
    run_cxx_path = os.path.join(os.environ['ANALYSIS_PATH'], 'Core', 'python', 'run_cxx.py')
    sm_path = os.path.join(os.environ['ANALYSIS_PATH'], 'Preprocessing', 'SimpleMergeMix.cxx')
    command = [ 'python', '-u', run_cxx_path, sm_path,
      '--cfg'               , str(self.cfg),
      '--input'             , str(self.input_path),
      '--output'            , tmp_file,
      '--input-spec'        , str(self.input_spec),
      '--n-jobs'            , str(self.n_jobs),
      '--job-idx'           , str(self.branch),
    ]
    optional_arguments = [
      ['--seed'              , str(self.seed)                    ],
      ['--prefix'            , str(self.prefix)                  ],
      ['--n-threads'         , str(self.n_threads)               ],
      ['--disabled-branches' , str(self.disabled_branches)       ],
      ['--compression-algo'  , str(self.compression_algo)        ],
      ['--compression-level' , str(self.compression_level)       ],
      ['--max-entries'       , str(self.max_entries)             ],
    ]
    for arg in optional_arguments:
      if len(arg[1]) > 0:
        command += arg
    sh_call(command, verbose=1)

    self.move(tmp_file, self.output().path)
    self.move('out', os.path.join(hash_output, f'out_{self.branch}'))
    print(f'Output file and hash tables moved to {self.output().path}')
