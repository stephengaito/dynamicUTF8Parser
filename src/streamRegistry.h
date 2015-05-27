#ifndef STREAM_REGISTRY_H
#define STREAM_REGISTRY_H

#include "utf8chars.h"

/// \brief A registry/collection of UTF8 character streams to be used
/// by a given forest of Parse Trees.
///
/// A stream registry is used to provide a coherent life time for a
/// collection of C-strings.
class StreamRegistry {
  public:

    /// \brief Create a new stream registry.
    StreamRegistry(void);

    /// \brief Destroy a stream registry.
    ///
    /// NOTE: all streams registered with a stream registry are *owned*
    /// by the registry.  They have the same lifetime, and will be
    /// destroyed if and when the registry itself is destroyed.
    ~StreamRegistry(void);

    /// \brief Add a C-string as a stream to this stream registry.
    ///
    /// Internally a Utf8Chars instance will be created with the given
    /// ownership models. The associated C-string will be freed depending
    /// upon the given owernship model.
    void addStream(const char *someUtf8Chars,
                   Utf8Chars::Ownership ownership = Utf8Chars::DoNotOwn);

    /// \brief Add a Utf8Chars instance to this stream registry.
    ///
    /// *NOTE:* This Utf8Chars instance will be destroyed when the
    /// stream registry itself is destroyed.
    void addStream(Utf8Chars *someUtf8Chars);

  private:

    /// \brief The collection of Ut8Chars streams.
    Utf8Chars **streams;

    /// \brief The index, in the streams array, of the next Utf8Chars
    /// pointer to be added to the streams array.
    size_t nextStreamToAdd;

    /// \brief The (current) size of the streams array.
    size_t numPossibleStreams;
};


#endif
