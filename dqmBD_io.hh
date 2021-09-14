// P.~Mandrik, 2021, https://github.com/pmandrik/dqmBD

#ifndef dqmBD_io_HH 
#define dqmBD_io_HH 1 

namespace dqmBD {

  enum {
    IO_ZERO   = -1,
    IO_FAILED,
    IO_OPENED,
  };

  class IO {
    public:
    int status = IO_ZERO;
    float file_size = -1;

    string GetSize(){
      if( file_size <= 1024 ) return to_string(file_size) + " B";
      if( file_size <= 1024*1024 ) return to_string(file_size/1024) + " kB";
      if( file_size <= 1024*1024*1024 ) return to_string(file_size/1024/1024) + " MB";
      return to_string(file_size/1024/1024/1024) + " GB";
    }

    bool LoadFile( const string & path, const string & name ){ 
      ifstream in_file((path + name).c_str(), ios::binary);
      in_file.seekg(0, ios::end);
      file_size = in_file.tellg();

      TFile * file = TFile::Open( (path + name).c_str() );
      status = IO_OPENED;
      if(!file or file->IsZombie()){
        cout << __FUNCTION__ << ": can't open file name \"" << name << "\", skip ... " << endl;
        status = IO_FAILED;
        return false;
      }

      LoadDir( file , "" );

      cout << "  Load:" << endl;
      cout << "    TH1Fs " << th1f_keys.size() << endl;

      file->Close();
      return true;
    }

    vector<string> th1f_keys;
    void LoadDir(TDirectory *source, string path) {
      // source->ls();
      TDirectory *savdir = gDirectory;
      TKey *key;
      TIter nextkey(source->GetListOfKeys(),kIterBackward);
      while ((key = (TKey*)nextkey())) {
        const char *classname = key->GetClassName();
        TClass *cl = gROOT->GetClass(classname);

        // cout << path << " \"" << key->GetName() << "\" " << key->GetClassName() << endl;

        if (!cl) continue;
        if (cl->InheritsFrom(TDirectory::Class())) {
          source->cd(key->GetName());
          TDirectory *subdir = gDirectory;
          LoadDir(subdir, path + "\\" + key->GetName() );
        } else if (cl->InheritsFrom(TTree::Class())) {}
        else if (key->GetClassName() == string("TH1F")) {
          cout << path << " \"" << key->GetName() << "\" " << key->GetClassName() << endl;
          th1f_keys.push_back( path + "\\" + key->GetName() );
        }
        else {
          // TObject *obj = key->ReadObj();
        }
      }
    }
  };

  vector<string> get_files( string path, string ext = ".root", bool verbose = false) {
    cout << __PRETTY_FUNCTION__ << " ... call " << endl;
    vector<string> answer;
    TSystemDirectory dir(path.c_str(), path.c_str()); 
    TList *files = dir.GetListOfFiles(); 
    if (files) { 
      TSystemFile *file; 
      TString fname; 
      TIter next(files); 
      while ((file=(TSystemFile*)next())) { 
        fname = file->GetName(); 
        if (!file->IsDirectory() && fname.EndsWith(ext.c_str())) { 
          if(verbose) cout << "  add " << fname.Data() << endl; 
          answer.push_back( fname.Data() );
        } else {
          if(verbose) cout << "  skip " << fname.Data() << endl; 
        }
      } 
    }

    return answer;
  }

};

void dqmBD_io(){
  string path = "/home/pmandrik/work/DQM/DQM_bd/";
  vector<string> files = dqmBD::get_files( path );
  for(string fname : files){
    dqmBD::IO loader;
    loader.LoadFile( path, fname );
    cout << fname << " " << loader.status << " " << loader.GetSize() << endl;
  }
}

#endif


