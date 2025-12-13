#!/bin/bash

#echo "Arguments received: $@"
#echo "Number of arguments: $#"

# Check for the required arguments
if [ $# -lt 4 ]; then
  echo "Usage: $0 <input_dir> <output_file> <output_entries> <file_number>"
  exit 1
fi

input_dir=$1
output_file=$2
output_entries=$3
file_number=$4


echo "Received arguments: $@"
echo "input_dir: $input_dir"
echo "output: $output_file"
echo "output_entries: $output_entries"
echo "file_number is : $file_number"

output_with_process="${output_file}.${file_number}"
output_entries_with_process="${output_entries}.${file_number}"

# Source the environment setup script
#source env_setup.sh

echo "initially in this directory : " $PWD

#ls $PWD

#echo "ls of condor is done"

WORKDIR=$PWD

#export X509_USER_PROXY=/afs/cern.ch/user/p/ppalit/.globus/x509up_u104629

voms-proxy-info --all || { echo "Proxy is invalid"; exit 1; }

sample_name=$(echo "$input_dir" | awk -F'/' '{print $6}')

echo "sample name : " $sample_name

xrdcp root://cms-xrd-global.cern.ch/$input_dir $PWD/tmp/inputntuple_${sample_name}_${file_number}.root

echo "xrdcp is done "

#ls -ltrh $WORKDIR/tmp/

JOBDIR=/afs/cern.ch/work/p/ppalit/public/displacedtauML/TauMLTools/
cd $JOBDIR
source ./env.sh cmssw

#export X509_USER_PROXY=/afs/cern.ch/user/p/ppalit/.globus/x509up_u104629

#####////export X509_USER_PROXY=/afs/cern.ch/user/p/ppalit/x509up_u104629



echo "Currently in this directory : " $PWD
chmod +x $PWD/Core/python/run_cxx.py

#ls $PWD

#echo "ls of jobdir is done"

# Run the `run_cxx` command with the passed arguments
python $PWD/Core/python/run_cxx.py $PWD/Preprocessing/CreateSpectralHists.cxx \
  --output "$output_with_process" \
  --output_entries "$output_entries_with_process" \
  --input-dir "${WORKDIR}/tmp/inputntuple_${sample_name}_${file_number}.root" \
  --mode jet \
  --n-threads 11

#  --input-dir "${WORKDIR}/tmp/inputntuple.root" \

#-input-dir "$input_dir" \

# Check the exit status of the run
if [ $? -eq 0 ]; then
    echo "run_cxx completed successfully"
else
    echo "run_cxx failed"
    exit 1
fi

ls -ltrh $JOBDIR

# Copy the output to EOS
#echo "Copying the output to EOS..."
#xrdcp -f "$output_with_process" root://eosuser.cern.ch//eos/user/p/ppalit2/Exotic/displacetauml/spectrum_ntuples/
#xrdcp -f "$output_entries_with_process" root://eosuser.cern.ch//eos/user/p/ppalit2/Exotic/displacetauml/spectrum_ntuples/

# Clean up (optional)
echo "Cleaning up local input files..."
#rm -f "$output_with_process" "$output_entries_with_process"

rm -f ${WORKDIR}/tmp/inputntuple_${sample_name}_${file_number}.root

echo "Done!"

