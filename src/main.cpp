#include <boost/program_options.hpp>
#include <iostream>
#include <sys/ioctl.h>
#include <yaml-cpp/yaml.h>

int main(int argc, char *argv[])
{
  using namespace std;
  namespace po = boost::program_options;

  const string caption = "partman v0.2";

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  po::options_description desc(caption, w.ws_col);
  desc.add_options()
    ("help,h",
     "display this help and exit")
    ("version,v",
     "output version information and exit")
  ;

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
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

  return EXIT_SUCCESS;
}
