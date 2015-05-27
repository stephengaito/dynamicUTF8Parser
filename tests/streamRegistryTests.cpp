#include <bandit/bandit.h>
using namespace bandit;

#include <string.h>
#include <stdio.h>
#include <exception>

#ifndef private
#define private public
#endif

#include <streamRegistry.h>

go_bandit([](){

  printf("\n----------------------------------\n");
  printf(  "StreamRegistry\n");
  printf(  "StreamRegistry = %zu bytes (%zu bits)\n", sizeof(StreamRegistry), sizeof(StreamRegistry));
  printf(  "----------------------------------\n");

  /// \brief We test the correctness of the StreamRegistry class.
  describe("StreamRegistry", [](){

    it("Should be able to create a stream registry", [](){
      StreamRegistry *someStreams = new StreamRegistry();
      AssertThat(someStreams, Is().Not().EqualTo((void*)0));
      AssertThat(someStreams->streams, Equals((void*)0));
      AssertThat(someStreams->nextStreamToAdd, Equals(0));
      delete someStreams;
    });

    it("Should be able to add streams to the registry", [](){
      StreamRegistry *someStreams = new StreamRegistry();
      AssertThat(someStreams, Is().Not().EqualTo((void*)0));
      AssertThat(someStreams->streams, Equals((void*)0));
      AssertThat(someStreams->nextStreamToAdd, Equals(0));
      const char* cString0 = "silly";
      someStreams->addStream(cString0);
      AssertThat(someStreams->streams, Is().Not().EqualTo((void*)0));
      AssertThat(someStreams->nextStreamToAdd, Equals(1));
      AssertThat(someStreams->streams[0]->utf8Chars, Equals((char*)cString0));
      AssertThat(someStreams->streams[0]->ownsString, Is().False());
      someStreams->addStream(cString0, Utf8Chars::Duplicate);
      AssertThat(someStreams->nextStreamToAdd, Equals(2));
      AssertThat(someStreams->streams[1]->utf8Chars,
                 Is().Not().EqualTo((char*)cString0));
      AssertThat(someStreams->streams[1]->ownsString, Is().True());
      char *cString1 = strdup("sillier");
      someStreams->addStream(cString1, Utf8Chars::TakeOwnership);
      AssertThat(someStreams->nextStreamToAdd, Equals(3));
      AssertThat(someStreams->streams[2]->utf8Chars, Equals((char*)cString1));
      AssertThat(someStreams->streams[2]->ownsString, Is().True());
      const char* cString2 = "silliest";
      Utf8Chars *aUtf8Char = new Utf8Chars(cString2);
      someStreams->addStream(aUtf8Char);
      AssertThat(someStreams->nextStreamToAdd, Equals(4));
      AssertThat(someStreams->streams[3], Equals((Utf8Chars*)aUtf8Char));
      AssertThat(someStreams->streams[3]->utf8Chars, Equals((char*)cString2));
      AssertThat(someStreams->streams[3]->ownsString, Is().False());
      delete someStreams;
    });

    it("should be able to add lots of streams", [](){
      Utf8Chars *prevChars[30];
      StreamRegistry *someStreams = new StreamRegistry();
      AssertThat(someStreams, Is().Not().EqualTo((void*)0));
      AssertThat(someStreams->streams, Equals((void*)0));
      AssertThat(someStreams->nextStreamToAdd, Equals(0));
      for (size_t i = 0; i < 25; i++) {
        someStreams->addStream("silly");
        AssertThat(someStreams->streams, Is().Not().EqualTo((void*)0));
        AssertThat(someStreams->nextStreamToAdd, Equals(i+1));
        prevChars[i] = someStreams->streams[i];
        for (size_t j = 0; j < i; j++) {
          AssertThat(someStreams->streams[j],
                     Equals((Utf8Chars*)prevChars[j]));
        }
      }
      delete someStreams;
    });

  }); // describe ParseTrees

});


