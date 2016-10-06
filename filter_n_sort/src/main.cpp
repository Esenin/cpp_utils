#include <assert.h>
#include <iostream>

#include "application.h"
#include "simple_filter.h"
#include "simple_storage.h"

namespace {
const char *simple_cmd = "--simple";
}

void PrintUsage() {
  std::cout << "The program filters a given file removing the exactly matched word and then performs"
      << " case-insensitive sort\n"
      << "Usage:\n"
      << "\t./filter_n_sort word in_file.txt [out_file.txt] [" << simple_cmd <<"]\n"
      << "\t./filter_n_sort --help\n"
      << "\t\t" << simple_cmd << "\t:: use simple implementation (may be a little slower) (optional)\n"
      << "\t\t--help\t:: show this message\n\n"
      << "NOTES: the program is NOT oriented to process files with bigger size than available memory\n";

}

app::Application ConfigureApp(const std::string &in_filename, const std::string &out_filename,
                              const std::string &filtered_word, bool simple_mode) {
  assert(simple_mode); // TODO: turn on other impl
  auto filter = app::UniqFilter(simple_mode? new app::SimpleFilter(in_filename, filtered_word) : nullptr);
  auto storage = app::UniqStorage(simple_mode? new app::SimpleStorage(out_filename) : nullptr);
  return app::Application(std::move(filter), std::move(storage));
}

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 5) {
    PrintUsage();
    return 0;
  }
  std::string filtered_word = argv[1];
  std::string input_file = argv[2];
  bool simple_mode = 0 == strcmp(argv[argc - 1], simple_cmd);
  std::string output_file = (argc == 5 || (argc == 4 && !simple_mode))? argv[3] : "out_" + input_file;

  auto application = ConfigureApp(input_file, output_file, filtered_word, simple_mode);

  application.Run();

  return 0;
}


