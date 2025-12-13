/*! Base class to merge multiple root files into a single file.
This file is part of https://github.com/cms-tau-pog/TauMLTools. */

#pragma once

#include <iostream>
#include <unordered_map>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TSystem.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1.h>

#include "RootExt.h"
#include "TextIO.h"
#include "TSystem.h"

bool isRunningInCondor() {
  const char* condorSlot = std::getenv("_CONDOR_SLOT");
    
    /*    if (condorJobId != nullptr ) {
        return true;
    } else {
        return false;
	}*/

    return (condorSlot != nullptr);
}


namespace analysis {

namespace detail {
inline void CollectInputFiles(const boost::filesystem::path& dir, std::vector<std::string>& files,
                              const boost::regex& pattern, const std::set<std::string>& exclude,
                              const std::set<std::string>& exclude_dirs)
{
  for(const auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dir), {})) {
    if(boost::filesystem::is_directory(entry) && !exclude_dirs.count(entry.path().filename().string()))
      CollectInputFiles(entry.path(), files, pattern, exclude, exclude_dirs);
    else if(boost::regex_match(entry.path().string(), pattern) && !exclude.count(entry.path().filename().string()))
      files.push_back(entry.path().string());
  }
}
  
  void CollectInputFilesXRootD(const std::string& dir, std::vector<std::string>& files,
			       const boost::regex& pattern, const std::set<std::string>& exclude,
			       const std::set<std::string>& exclude_dirs)
  {
    // Command to list files in XRootD directory using xrdfs
    
    //std::string command = "xrdfs " + dir + " ls";
    //std::cout << "Executing command: " << command << std::endl;
    
    std::cout<< "If running condor : " << isRunningInCondor() << std::endl;

    size_t pos = dir.find("//store");
    std::string prefix = dir.substr(0, pos + 1);   
    std::string pathdir = dir.substr(pos + 1);     
    
    std::string command = "xrdfs " + prefix + " ls " + pathdir;
    
    std::cout << "prefix is : " << prefix << std::endl;
    std::cout << "pathdir is : " << pathdir << std::endl;

    std::cout << "Executing command: " << command << std::endl;

    std::string command_with_output_and_error_redirect = command + " 2>&1";
    
    // Execute command and capture the output
    /////#####FILE* pipe = gSystem->OpenPipe(command.c_str(), "r");
    FILE* pipe = gSystem->OpenPipe(command_with_output_and_error_redirect.c_str(), "r");


    /*    FILE* openPipeWithRetry(const std::string& command, int max_retries = 50, int retry_delay = 2) {
      FILE* pipe = nullptr;
      int attempt = 0;
      
      while (attempt < max_retries) {
        pipe = gSystem->OpenPipe(command.c_str(), "r");
        if (pipe) {
            return pipe;  // Success, return the pipe
        }
	
        // If failed, print retry attempt and wait
        std::cerr << "Attempt " << (attempt + 1) << " failed, retrying in " << retry_delay << " seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(retry_delay));
        ++attempt;
      }
      
      throw std::runtime_error("Failed to execute xrdfs ls after retries");
      }*/
    
    std::cout<< "After OpenPipe command" << std::endl; 
    
    if (!pipe){
      std::cerr << "Error opening pipe for command: " << command << std::endl;
      perror("OpenPipe error"); 
      throw std::runtime_error("Failed to execute xrdfs ls");
      
    }
      
    std::cout<< "After xrdfs failed command" << std::endl;
    
    char buffer[256];
    std::vector<std::string> file_list;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      std::cout<< "startting the buffer loop" << std::endl;
      std::string file = buffer;
      std::cout<< "going to trim in the buffer loop" << std::endl;
      boost::trim(file);

      std::cout << "Command Output: " << file << std::endl;
      
      // Skip directories that are in the exclude list
      size_t last_slash = file.find_last_of('/');
      if (last_slash != std::string::npos) {
	std::string dir_name = file.substr(last_slash + 1); 
	
	std::cout << "The directory name is : " << dir_name << std::endl;
	
	if (boost::filesystem::is_directory(file) && exclude_dirs.count(dir_name)) {
	  continue; 
	}
      } else {
	std::cerr << "Warning: Skipping unexpected warning line: " << file << std::endl;
	continue;  
      }

      
      /*std::string dir_name = file.substr(last_slash + 1);
      
      std::cout << "The directory name is : " << dir_name << std::endl;
      
      if (boost::filesystem::is_directory(file) && exclude_dirs.count(dir_name)) {
	continue;
	}*/
      
      file_list.push_back(file);
    }
    gSystem->ClosePipe(pipe);
    
    // Apply the pattern matching and exclusion
    for (const auto& file : file_list) {
      if (boost::regex_match(file, pattern) && !exclude.count(file)) {
	std::string full_file_path = prefix + '/' + file;
	std::cout << "second prefix name is : " << prefix << std::endl;
	std::cout << "file is : " << file << std::endl;
	
	files.push_back(full_file_path);
      }
    }
  }

  
  /*  
  void CollectInputFilesXRootDCondor(const std::string& dir, std::vector<std::string>& files)
  {
    // Command to list files in XRootD directory using xrdfs
    
    //std::string command = "xrdfs " + dir + " ls";
    //std::cout << "Executing command: " << command << std::endl;
    
    std::cout<< "If running condor : " << isRunningInCondor() << std::endl;

    size_t pos = dir.find("//store");
    std::string prefix = dir.substr(0, pos + 1);   
    std::string pathdir = dir.substr(pos + 1);     
    
    std::string command = "xrdfs " + prefix + " ls " + pathdir;
    
    std::cout << "prefix is : " << prefix << std::endl;
    std::cout << "pathdir is : " << pathdir << std::endl;

    std::cout << "Executing command: " << command << std::endl;

    std::string command_with_output_and_error_redirect = command + " 2>&1";
    
    // Execute command and capture the output
    /////#####FILE* pipe = gSystem->OpenPipe(command.c_str(), "r");
    FILE* pipe = gSystem->OpenPipe(command_with_output_and_error_redirect.c_str(), "r");

    
    std::cout<< "After OpenPipe command" << std::endl; 
    
    if (!pipe){
      std::cerr << "Error opening pipe for command: " << command << std::endl;
      perror("OpenPipe error"); 
      throw std::runtime_error("Failed to execute xrdfs ls");
      
    }
      
    std::cout<< "After xrdfs failed command" << std::endl;
    
    char buffer[256];
    std::vector<std::string> file_list;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      std::cout<< "startting the buffer loop" << std::endl;
      std::string file = buffer;
      std::cout<< "going to trim in the buffer loop" << std::endl;
      boost::trim(file);

      std::cout << "Command Output: " << file << std::endl;
      
      // Skip directories that are in the exclude list
      size_t last_slash = file.find_last_of('/');
      if (last_slash != std::string::npos) {
	std::string dir_name = file.substr(last_slash + 1); 
	
	std::cout << "The directory name is : " << dir_name << std::endl;
	
	if (boost::filesystem::is_directory(file) && exclude_dirs.count(dir_name)) {
	  continue; 
	}
      } else {
	std::cerr << "Warning: Skipping unexpected warning line: " << file << std::endl;
	continue;  
      }

      
    
      
      file_list.push_back(file);
    }
    gSystem->ClosePipe(pipe);

    
    
    // Apply the pattern matching and exclusion
    for (const auto& file : file_list) {
      if (boost::regex_match(file, pattern) && !exclude.count(file)) {
	std::string full_file_path = prefix + '/' + file;
	std::cout << "second prefix name is : " << prefix << std::endl;
	std::cout << "file is : " << file << std::endl;
	
	files.push_back(full_file_path);
      }
    }
  }
  */
  
} // namespace detail
  
class RootFilesMerger {
public:
  struct HistDescriptor {
    static constexpr size_t MergeThreshold = 20;
    using HistPtr = std::unique_ptr<TH1>;
    std::vector<HistPtr> hists;
    HistDescriptor() { hists.reserve(MergeThreshold + 1); }
    void AddHistogram(HistPtr&& new_hist)
    {
      hists.push_back(std::move(new_hist));
      if(hists.size() > MergeThreshold)
        Merge();
    }
    const HistPtr& GetMergedHisto() const
    {
      if(hists.size() != 1)
        throw analysis::exception("Merged histogram is not ready");
      return hists[0];
    }
    void Merge()
    {
      if(hists.size() <= 1) return;
      {
        TList list;
        for(size_t n = 1; n < hists.size(); ++n)
          list.Add(hists[n].get());
        hists[0]->Merge(&list);
      }
      hists.resize(1);
    }
  };

  struct TreeDescriptor {
    using ChainPtr = std::unique_ptr<TChain>;
    static std::atomic<size_t>& NumberOfFiles()
    {
      static std::atomic<size_t> n_files = 1;
      return n_files;
    }
    std::vector<std::string> file_names;
    TreeDescriptor() { file_names.reserve(NumberOfFiles()); }
    void AddFile(const std::string& file_name) { file_names.push_back(file_name); }
    ChainPtr CreateChain(const std::string& full_name) const
    {
      auto chain = std::make_unique<TChain>(full_name.c_str());
      for(const auto& file_name : file_names)
        chain->AddFile(file_name.c_str());
      return chain;
    }
  };

  struct Key {
    std::string dir_name, name, full_name;
    Key() {}
    Key(const std::string& _dir_name, const std::string& _name) :
        dir_name(_dir_name), name(_name), full_name(dir_name + name) {}
    bool operator==(const Key& other) const { return full_name == other.full_name; }
    bool operator<(const Key& other) const { return full_name < other.full_name; }
  };

  struct KeyHash {
    size_t operator()(const Key& key) const { return h(key.full_name); }
  private:
    std::hash<std::string> h;
  };

  using HistCollection = std::unordered_map<Key, HistDescriptor, KeyHash>;
  using TreeCollection = std::unordered_map<Key, TreeDescriptor, KeyHash>;

  struct ObjectCollection {
    HistCollection hists;
    TreeCollection trees;
  };

  using HistPtr = HistDescriptor::HistPtr;
  using ChainPtr = TreeDescriptor::ChainPtr;

  RootFilesMerger(const std::string& output, const std::vector<std::string>& input_dirs,
                  const std::string& file_name_pattern, const std::string& exclude_list,
                  const std::string& exclude_dir_list, unsigned n_threads, ROOT::ECompressionAlgorithm compression,
                  int compression_level)
    : input_files(FindInputFiles(input_dirs, file_name_pattern, exclude_list, exclude_dir_list)),
      output_file(root_ext::CreateRootFile(output, compression, compression_level))
  {
    TreeDescriptor::NumberOfFiles() = input_files.size();
    if(n_threads > 1)
      ROOT::EnableImplicitMT(n_threads);
  }

  virtual ~RootFilesMerger() {}

  void Process(bool process_histograms, bool process_trees)
  {
    for(const auto& file_name : input_files) {
      std::cout << "file: " << file_name << std::endl;
      auto file = root_ext::OpenRootFile(file_name);
      ProcessDirectory(file_name, "", file.get(), objects, process_histograms, process_trees);
      ProcessFile(file_name, file);
    }

    if(process_histograms) {
      std::cout << "Writing histograms..." << std::endl;
      {
        std::map<Key, const HistDescriptor*> ordered_histograms;
        for(auto& hist_entry : objects.hists) {
          hist_entry.second.Merge();
          ordered_histograms[hist_entry.first] = &hist_entry.second;
        }

        for(auto& hist_entry : ordered_histograms) {
          auto dir = root_ext::GetDirectory(*output_file, hist_entry.first.dir_name);
          root_ext::WriteObject(*hist_entry.second->GetMergedHisto(), dir);
        }
        objects.hists.clear();
      }
    }

    if(process_trees) {
      std::map<Key, const TreeDescriptor*> ordered_trees;
      for(auto& tree_entry : objects.trees)
        ordered_trees[tree_entry.first] = &tree_entry.second;
      for(auto& tree : ordered_trees) {
        std::cout << "tree: " << tree.first.full_name << std::endl;
        auto dir = root_ext::GetDirectory(*output_file, tree.first.dir_name);
        Long64_t n_entries = -1;
        dir->cd();
        {
          auto chain = tree.second->CreateChain(tree.first.full_name);
          n_entries = chain->GetEntries();
          chain->Merge(output_file.get(), 0, "C keep");
        }
        std::unique_ptr<TTree> merged_tree(root_ext::ReadObject<TTree>(*output_file, tree.first.full_name));
        if(merged_tree->GetEntries() != n_entries)
          throw analysis::exception("Not all files were merged for '%1%' tree.") % tree.first.full_name;
      }
    }
  }

  /*  static std::vector<std::string> FindInputFiles(const std::vector<std::string>& dirs,
                                                 const std::string& file_name_pattern,
                                                 const std::string& exclude_list,
                                                 const std::string& exclude_dir_list)
  {
    auto exclude_vector = analysis::SplitValueList(exclude_list, true, ",");
    std::set<std::string> exclude(exclude_vector.begin(), exclude_vector.end());

    auto exclude_dir_vector = analysis::SplitValueList(exclude_dir_list, true, ",");
    std::set<std::string> exclude_dirs(exclude_dir_vector.begin(), exclude_dir_vector.end());

    const boost::regex pattern(file_name_pattern);
    std::vector<std::string> files;
    for(const auto& dir : dirs) {
      boost::filesystem::path path(dir);
      detail::CollectInputFiles(path, files, pattern, exclude, exclude_dirs);
    }
    return files;
  }
  */

  static std::vector<std::string> FindInputFiles(const std::vector<std::string>& dirs,
                                               const std::string& file_name_pattern,
                                               const std::string& exclude_list,
                                               const std::string& exclude_dir_list)
  {
    auto exclude_vector = analysis::SplitValueList(exclude_list, true, ",");
    std::set<std::string> exclude(exclude_vector.begin(), exclude_vector.end());
    
    auto exclude_dir_vector = analysis::SplitValueList(exclude_dir_list, true, ",");
    std::set<std::string> exclude_dirs(exclude_dir_vector.begin(), exclude_dir_vector.end());
    
    const boost::regex pattern(file_name_pattern);
    std::vector<std::string> files;

    std::cout<< "Here going to collect input files.."<< std::endl;
    
    for(const auto& dir : dirs) {
      if (isRunningInCondor()){
	///###std::string prefix = "root://cms-xrd-global.cern.ch/";
	///###std::string xrdpathdir = prefix + dir;
	std::string xrdpathdir = dir;
	files.push_back(xrdpathdir);
      }
      else{
      if (boost::starts_with(dir, "root://")) {
	// Handle XRootD directories
	detail::CollectInputFilesXRootD(dir, files, pattern, exclude, exclude_dirs);
      } else {
	// Handle local directories
	boost::filesystem::path path(dir);
	detail::CollectInputFiles(path, files, pattern, exclude, exclude_dirs);
      }
      }
    }
    
    return files;
  }
  
private:
  virtual void ProcessFile(const std::string& /*file_name*/, const std::shared_ptr<TFile>& /*file*/) {}

  static void ProcessDirectory(const std::string& file_name, const std::string& dir_name, TDirectory* dir,
                               ObjectCollection& objects, bool process_histograms, bool process_trees)
  {
    std::cout << "Entering ProcessDirectory for directory: " << dir_name
              << " from file: " << file_name << std::endl;
    using ClassInheritance = root_ext::ClassInheritance;
    TIter nextkey(dir->GetListOfKeys());
    for(TKey* t_key; (t_key = dynamic_cast<TKey*>(nextkey()));) {
      std::cout << "Processing key: " << t_key->GetName()
              << " of class: " << t_key->GetClassName() << std::endl;
      const ClassInheritance inheritance = root_ext::FindClassInheritance(t_key->GetClassName());
      const Key key(dir_name, t_key->GetName());

      switch (inheritance) {
        case ClassInheritance::TH1: {
          if(process_histograms) {
	    std::cout << "Processing histogram: " << key.name << " in directory: " << dir_name << std::endl;
            auto hist = HistPtr(root_ext::ReadObject<TH1>(*dir, key.name));
            hist->SetDirectory(nullptr);
            objects.hists[key].AddHistogram(std::move(hist));
          }
          break;
        }
        case ClassInheritance::TTree: {
          if(process_trees) {
	    std::cout << "Processing tree: " << key.name << " in directory: " << dir_name << std::endl;
	    objects.trees[key].AddFile(file_name);
          }
          break;
        } case ClassInheritance::TDirectory: {
          auto subdir = root_ext::ReadObject<TDirectory>(*dir, key.name);
	  std::cout << "Recursively calling ProcessDirectory for subdirectory: " << key.full_name << std::endl;
          ProcessDirectory(file_name, key.full_name + "/", subdir, objects, process_histograms,
                          process_trees);
          break;
        }
      }
    }
  }

protected:
  const std::vector<std::string> input_files;
  std::shared_ptr<TFile> output_file;
  ObjectCollection objects;
};

} // namespace analysis
