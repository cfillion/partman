#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sys/ioctl.h>
#include <yaml-cpp/yaml.h>

#include "generators.hpp"

using namespace std;
using namespace boost;

bool process(const string &file)
{
  cerr << format("Pre-Processing '%s'") % file << endl;

  Document doc;

  try {
    vector<YAML::Node> yaml;

    if(file == "-")
      yaml = YAML::LoadAll(cin);
    else
      yaml = YAML::LoadAllFromFile(file);

    for(YAML::Node &root : yaml)
      doc.read_yaml(root);
  }
  catch(std::exception &err) {
    cerr << format("%s: %s") % file % err.what() << endl;
    return false;
  }

  cout << doc.token();

  return true;
}

int main(int argc, char *argv[])
{
  namespace po = program_options;

  const string caption = "partman v0.2";

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  const vector<string> default_files{"-"};

  po::options_description desc(caption, w.ws_col);
  desc.add_options()
    ("input,i", po::value<vector<string> >()->value_name("FILE")
      ->default_value(default_files, "-"), "list of files to process")

    ("help,h",
     "display this help and exit")
    ("version,v",
     "output version information and exit")
  ;

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
      .positional(p).run(), vm);

    po::notify(vm);
  }
  catch(po::error &err) {
    cerr << err.what() << endl << endl;
    cout << desc;
    return EXIT_FAILURE;
  }

  if(vm.count("help")) {
    cout << desc;
    return EXIT_SUCCESS;
  }

  if(vm.count("version")) {
    cout << caption << endl;
    return EXIT_SUCCESS;
  }

  for(const string file : vm["input"].as<vector<string> >())
    process(file);

  return EXIT_SUCCESS;
}
