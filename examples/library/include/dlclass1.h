#ifndef DLCLASS1_H
#define DLCLASS1_H

namespace dummyLibOne {

class class1 {
 public:
  int function1(int x);
  int function2(int y);
};

}  // namespace dummyLibOne

namespace DummyVnV {
void Initialize(int argc, char** argv, const char* filename);
void Finalize();
}  // namespace DummyVnV

#endif
