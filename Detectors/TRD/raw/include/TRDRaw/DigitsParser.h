// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   EpnRawReaderTask.h
/// @author Sean Murray
/// @brief  TRD cru output data to tracklet task

#ifndef O2_TRD_DIGITSPARSER
#define O2_TRD_DIGITSPARSER

#include "Framework/Task.h"
#include "Framework/DataProcessorSpec.h"
#include "TRDRaw/CruRawReader.h"
#include "TRDRaw/CompressedRawReader.h"
#include <fstream>

using namespace o2::framework;

namespace o2
{
namespace trd
{
class Digit;

class DigitsParser
{
 public:
  DigitsParser() = default;
  ~DigitsParser() = default;
  uint32_t Parse(std::vector<uint64_t>* data)
  {
    setData(data);
    return Parse();
  };
  uint32_t Parse();
  int setData(std::vector<uint64_t>* data) { mData = data; };

 private:
  std::vector<uint64_t>* mData = nullptr; // parsed in vector of raw data to parse.
  std::vector<Digit> mDigits;             // parsed in vector of raw data to parse.
  int mParsedWords{0};                    // words parsed in data vector, last complete bit is not parsed, and left for another round of data update.
};

} // namespace trd
} // namespace o2

#endif // O2_TRD_DIGITSPARSER