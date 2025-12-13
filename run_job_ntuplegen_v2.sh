#!/bin/bash
# run_job.sh

echo "Running on $(hostname)"
echo "Job process: $1"

# Setup CMSSW environment
#source /cvmfs/cms.cern.ch/cmsset_default.sh

#WORKDIR=$_CONDOR_SCRATCH_DIR
#cd $WORKDIR

JOBDIR=/afs/cern.ch/work/p/ppalit/public/displacedtauML/TauMLTools/
cd $JOBDIR


# Pick file from file_list.txt based on job index
FILE=$(sed -n "$(($1 + 1))p" $2)
echo "Processing file: $FILE"

#OUTPUT=output_${1}.root

BASENAME=$(basename "$FILE")
DATASET=$(echo "$FILE" | sed -E 's#.*/([^/]+)_Tune.*#\1#')
OUTPUT=${DATASET}_output_${1}.root

echo "Output file name: $OUTPUT"

export LD_LIBRARY_PATH=/usr/lib64:$LD_LIBRARY_PATH

echo "analysis path : $ANALYSIS_PATH"
source ./env.sh cmssw

env -i HOME=$HOME ANALYSIS_PATH=$ANALYSIS_PATH ANALYSIS_DATA_PATH=$ANALYSIS_DATA_PATH X509_USER_PROXY=$X509_USER_PROXY DEFAULT_CMSSW_BASE=$DEFAULT_CMSSW_BASE $ANALYSIS_PATH/RunKit/cmsEnv.sh cmsRun $ANALYSIS_PATH/Production/python/Production.py sampleType=MC era=Run2_2018 inputFiles=$FILE output=$OUTPUT maxEvents=2000 disabledBranches="boostedTau_.*,fatJet_.*"



# Run ROOT / CMSSW processing
#root -l -b -q "process_file.C(\"$FILE\",\"output_${1}.root\")"

# Move output to EOS
OUTDIR=/eos/user/p/ppalit2/Exotic/displacetauml/DYJets_condor_ntuples
#mkdir -p $OUTDIR
#mv output_${1}.root $OUTDIR
mv $OUTPUT $OUTDIR
