#include <vector>
#include <string>
#include <memory>
#include <cstdint>

#include <iostream>
#include <filesystem>

struct FileNode{
  std::filesystem::path name;
  std::uintmax_t size = 0;
  bool is_directory = false;
  std::vector<std::shared_ptr<FileNode>> children;
};

std::shared_ptr<FileNode> scanDirectory(std::shared_ptr<FileNode> root){

  std::filesystem::path Path(root->name);

  try{
    auto options = std::filesystem::directory_options::skip_permission_denied; // automatically skip folders with no permission to read from

    for(const auto& entry : std::filesystem::directory_iterator(Path)){

      if (entry.is_symlink()){
        continue; 
      }

      auto childNode = std::make_shared<FileNode>();
      childNode->name = entry.path();

      if(entry.is_directory()){
        childNode->is_directory = true;
        childNode = scanDirectory(childNode); // manual recursion
        root->children.push_back(childNode);
        root->size += childNode->size;
      }
      else{
        try{
          childNode->is_directory = false;
          childNode->size = std::filesystem::file_size(entry.path());

          root->children.push_back(childNode);
          root->size += childNode->size;
        }catch(std::filesystem::filesystem_error& e){
          std::cerr << "Error: " << e.what() << "\n";
        }
      }
    }
  }
  catch(const std::filesystem::filesystem_error &e){
    std::cerr << "Error: " << e.what() << "\n";
  }

  return root;
}

void printScan(std::shared_ptr<FileNode> root, int depth = 0, std::string sign = "──> "){

  for(int i = 0; i < depth ; i++){
    std::cout << "\t";
  }

  std::cout << sign << root->name.filename().string() << " : " << static_cast<double>(root->size) / (1024 * 1024)  << " MB"<< "\n";

  for(const auto& child : root->children){
    if(child == (root->children).back()){
      printScan(child, depth + 1, "└── ");
    }
    else{
      printScan(child, depth + 1, "├── ");
    }
  }
}

int main(int argc, char* argv[]){

  if (argc > 2){
      std::cout << "Usage: " << argv[0] << " [directory path]\n";
      exit(64);
  }

  std::shared_ptr<FileNode> root(new FileNode);
  root->name = argc == 1 ? "." : argv[1];;
  root->is_directory = true;

  printScan(scanDirectory(root));

  std::uintmax_t dirs{0};
  for(const auto& entry : root->children){
    if(entry->is_directory){
      dirs+=1;
    }
  }

  std::cout << "Total directories: " << dirs << " Total size: " << root->size << "\n";

  return 0;
}