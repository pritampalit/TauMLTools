# Displaced Tau Tagging Documentation

This file contains instructions for reproducing the displaced tau tagging implemented within TauMLTools machinery.

## How to produce inputs for the Displaced Taus tagging

Steps below describe how to process input datasets starting from MiniAOD updated with the functionality for displaced tau tagging specific.

### Big root-tuples production

[Production/python/Production.py](https://github.com/cms-tau-pog/TauMLTools/blob/master/Production/python/Production.py) contains the configuration that allows to run `TauTupleProducer` with `cmsRun`.
The config script should be run with the specification of the `rerunTauReco` and `selector`. Also disabling unnecessary branches can be used for saving disk disk space. Here is an example how to run `TauTupleProducer` on 1000 long-lived SUSY-TAU events using one MiniAOD file as an input:
```sh
cmsRun ./TauMLTools/Production/python/Production.py sampleType=MC era=Run2_2018 inputFiles=/store/mc/RunIISummer20UL18MiniAODv2/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/MINIAODSIM/106X_upgrade2018_realistic_v16_L1v1-v2/120000/4AFA2D6F-F6CD-6C46-8962-13C479C00E8B.root maxEvents=1000 disabledBranches="boostedTau_.*,fatJet_.*" rerunTauReco="displacedTau"
```

In order to run a large-scale production for the entire datasets, the CMS computing grid should be used via CRAB interface.
Submission and status control can be performed using [crab_submit.py](https://github.com/cms-tau-pog/TauMLTools/blob/master/Production/scripts/crab_submit.py) and [crab_cmd.py](https://github.com/cms-tau-pog/TauMLTools/blob/master/Production/scripts/crab_cmd.py) commands.

#### UL2018 tuple production


The production is identical to the [2018 tuples production](https://github.com/cms-tau-pog/TauMLTools/edit/master/README.md#2018-root-tuple-production-steps).

The main change is related to the specifying additional function in config files:
`disabledBranches=boostedTau_.*,fatJet_.*` - allows to disable branches that are not used.
`rerunTauReco=displacedTau` - rerun jet clustering and reconstruction with modified IP criterion to be more sensitive to displaced taus/
`selector=TauJetTag` - selects events with genJet or recoJet or genLepton  


### Updated condor job submission (13/12/2025)

```
chmod +x submit_condor_ntuplegen_v2.sh
./submit_condor_ntuplegen_v2.sh filelist.txt
```
`filelist.txt` will contain the dataset files. Change `OUTDIR` in `run_job_ntuplegen_v2.sh`. 


### Merging the ntuples

#### Spectrum creation
This step merge and mix the training files.

Spectrum of the initial data is needed to calculate the probability to take tau candidate of some genuine tau-type and from some pt-eta bin. To generate the spectrum histograms for a dataset and lists with number of entries per datafile, run:
```sh
CreateSpectralHists --output "spectrum_file.root" \
                    --output_entries "entires_file.txt" \
                    --input-dir "path/to/dataset/dir" \
                    --pt-hist "n_bins_pt, pt_min, pt_max" \
                    --eta-hist "n_bins_eta, |eta|_min, |eta|_max" \
	        –mode “jet”
                    --n-threads 1
```
on this step it is important to have high granularity binning to be able later to re-bin into custom, non-uniform pt-eta bins on the next step.

**S&M spectrum is not used for the current version of SimpleMix script however to be considered in the further preprocessing step developments.**

Alternatively, if one wants to process several datasets the following python script can be used (parameters of pt and eta binning to be hardcoded in the script):
```sh
python Analysis/python/CreateSpectralHists.py --input /path/to/input/dir/ \
                                              --output /path/to/output/dir/ \
                                              --filter ".*(DY).*" \
                                              --rewrite
                                              --mode "jet"
```
Mode `jet` is added specifically for using displaced tau object definitions [here](Analysis/interface/DisTauTagSelection.h).

After the following step spectrums and .txt files with the number of entries will be created in the output folder. To merge all the .txt files into one and mix the lines:
```
cat <path_to_spectrums>/*.txt | shuf - > filelist_mix.txt
```
Mixing is needed to have shuffled files within one data group. After this step, filelist_mix.txt should contain filenames and the number of entries in the corresponding files:
```
./DYJetsToLL_M-50/eventTuple_1-70.root 249243
./TTJets_ext1/eventTuple_1-308.root 59414
./TTToHadronic/eventTuple_467.root 142781
./WJetsToLNu/eventTuple_25.root 400874
...
```

#### Simple mix:
After spectrums are created for all datasets, the final procedure of Shuffle and Merge can be performed with:
```sh
SimpleMergeMix --cfg Analysis/config/2018_UL_simpl/training_input_test.cfg
                --input filelist_mix.txt
                --output <path_to_output_file.root>
                --input-spec <path_to_spectrum>
                --n-jobs 1
                --job-idx 0
                --prefix <path_to_data>
                --disabled-branches "boostedTau_.*,fatJet_.*"

```
The plugin arguments are analogical to the [S&M](https://github.com/cms-tau-pog/TauMLTools#shuffling--merging).


### Updated condor submission while accessing remote input files (13/12/2025)

CHange the pt, eta binning in `CreateSpectralHists.cxx` : `pt_hist = "4990, 10, 5000"` and `eta_hist = "25, 0, 2.5"`

Showing example for one file : 

```
submit_condor_jobs_qcd.sh qcd_300_500_filelist.txt <output_dir>/qcd_300_500_spectrum.root <output_dir>/qcd_300_500_entries.txt
```
This script takes input file list txt file, output file name and output entries file name.
Similarly, `submit_condor_jobs_stau.sh`, `condorjob_stau_multi.jdl`, `condorjob_qcd_multi.jdl`. the difference between stau and qcd is in condor job flavor only. Please change the `JOBDIR` in `run_job_multi.sh`

Now, it will give : 
```
ls qcd_300_500_entries.txt.*
qcd_300_500_entries.txt.1
qcd_300_500_entries.txt.10
qcd_300_500_entries.txt.11
.....

```
and 

```
ls qcd_300_500_spectrum.root.*
qcd_300_500_spectrum.root.1
qcd_300_500_spectrum.root.10
qcd_300_500_spectrum.root.11
.....

```

This will combine all the text file
```
cat qcd_1500_2000_entries.txt.* > qcd_1500_2000_entries_final.txt
```

So, for some internal modifications, do this to get the right entries text file only if you access input files via xrootd for spectrum creation.

```
python /afs/cern.ch/work/p/ppalit/public/displacedtauML/TauMLTools/mergefiles_afterstep2condor.py qcd_1000_1500_filelist.txt qcd_1000_1500_entries_final.txt qcd_1000_1500_merged_entries_final.txt
```

`qcd_1000_1500_filelist.txt` is the input file list. 

To get the combined root files, do hadd. Follow a proper naming for the hadd output root files as shown below, which will be used as input spectrum to step 3 SimpleMergeMix 
```
QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
rhpt.root
lhpt.root
QCD_HT50to100_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8.root
SUS-RunIISummer20UL18GEN-stau250_lsp1_ctau1000mm_v4.root
SUS-RunIISummer20UL18GEN-stau250_lsp1_ctau100mm_v6.root
SUS-RunIISummer20UL18GEN-stau100_lsp1_ctau100mm_v6.root
SUS-RunIISummer20UL18GEN-stau100_lsp1_ctau1000mm_v4.root
SUS-RunIISummer20UL18GEN-stau400_lsp1_ctau100mm_v6.root
SUS-RunIISummer20UL18GEN-stau400_lsp1_ctau1000mm_v4.root
```

Make a final entries.txt 
```
cat entries_lhpt_final_new.txt entries_rhpt_final_new.txt stau_250_1000_entries_test.txt stau_400_1000_entries_test.txt stau_400_100_entries_test.txt stau_250_100_entries_test.txt stau_100_1000_entries_test.txt stau_100_100_entries_test.txt qcd_50_100_merged_entries_final.txt qcd_200_300_merged_entries_final.txt qcd_300_500_merged_entries_final.txt qcd_500_700_merged_entries_final.txt qcd_700_1000_merged_entries_final.txt qcd_1000_1500_merged_entries_final.txt qcd_1500_2000_merged_entries_final.txt qcd_2000_inf_merged_entries_final.txt | shuf - > filelist_mix.txt
```

## Step 3 : SimpleMergeMix (updated on 13/12/2025)

`LawWorkflows/SimpleMergeMix.py`

Use law and modfiy input cfg
```
law run SimpleMergeMix --version v1 --cfg Analysis/config/2018_UL_simpl/training_inputs_MC_training_samples_vsimplemix_condor.cfg --input-path filelist_mix.txt --output-path <your_preferred_output_dir> --n-jobs 15 --prefix root://cms-xrd-global.cern.ch// --input-spec <path_to_step2output_spectrum> --disabled-branches "boostedTau_.*,fatJet_.*"
```

Law output can be seen in `data/SimpleMergeMix/`

