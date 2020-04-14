// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file RawPixelDecoder.h
/// \brief Definition of the Alpide pixel reader for raw data processing
#ifndef ALICEO2_ITSMFT_RAWPIXELDECODER_H_
#define ALICEO2_ITSMFT_RAWPIXELDECODER_H_

#include <array>
#include <TStopwatch.h>
#include "Framework/Logger.h"
#include "ITSMFTReconstruction/ChipMappingITS.h"
#include "ITSMFTReconstruction/ChipMappingMFT.h"
#include "DetectorsRaw/HBFUtils.h"
#include "Headers/RAWDataHeader.h"
#include "CommonDataFormat/InteractionRecord.h"
#include "ITSMFTReconstruction/GBTLink.h"
#include "ITSMFTReconstruction/RUDecodeData.h"
#include "ITSMFTReconstruction/PixelReader.h"
#include "DataFormatsITSMFT/ROFRecord.h"
#include "ITSMFTReconstruction/PixelData.h"

namespace o2
{
namespace framework
{
class InputRecord;
}

namespace itsmft
{
class ChipPixelData;

template <class Mapping>
class RawPixelDecoder : public PixelReader
{
  using RDH = o2::header::RAWDataHeader;

 public:
  RawPixelDecoder();
  ~RawPixelDecoder() final;
  void init() final {}
  bool getNextChipData(ChipPixelData& chipData) final;
  ChipPixelData* getNextChipData(std::vector<ChipPixelData>& chipDataVec) final;

  void startNewTF(o2::framework::InputRecord& inputs);

  int decodeNextTrigger();
  int decodeNextTrigger(int il);

  template <class DigitContainer, class ROFContainer>
  int fillDecodedDigits(DigitContainer& digits, ROFContainer& rofs);

  const RUDecodeData* getRUDecode(int ruSW) const { return mRUEntry[ruSW] < 0 ? nullptr : &mRUDecodeVec[mRUEntry[ruSW]]; }

  void setVerbosity(int v);
  int getVerbosity() const { return mVerbosity; }

  bool getDecodeNextAuto() const { return mDecodeNextAuto; }
  void setDecodeNextAuto(bool v) { mDecodeNextAuto = v; }

  void printReport();

  TStopwatch& getTimerTFStart() { return mTimerTFStart; }
  TStopwatch& getTimerDecode() { return mTimerDecode; }
  TStopwatch& getTimerExtract() { return mTimerDecode; }
  uint32_t getNChipsFiredROF() const { return mNChipsFiredROF; }
  uint32_t getNPixelsFiredROF() const { return mNPixelsFiredROF; }
  size_t getNChipsFired() const { return mNChipsFired; }
  size_t getNPixelsFired() const { return mNPixelsFired; }

  struct LinkEntry {
    int entry = -1;
  };

 private:
  void setupLinks(o2::framework::InputRecord& inputs);
  int getRUEntrySW(int ruSW) const { return mRUEntry[ruSW]; }
  RUDecodeData* getRUDecode(int ruSW) { return &mRUDecodeVec[mRUEntry[ruSW]]; }
  GBTLink* getGBTLink(int i) { return i < 0 ? nullptr : &mGBTLinks[i]; }
  const GBTLink* getGBTLink(int i) const { return i < 0 ? nullptr : &mGBTLinks[i]; }
  RUDecodeData& getCreateRUDecode(int ruSW);

  static constexpr uint16_t NORUDECODED = 0xffff; // this must be > than max N RUs

  std::vector<GBTLink> mGBTLinks;                           // active links pool
  std::unordered_map<uint32_t, LinkEntry> mSubsSpec2LinkID; // link subspec to link entry in the pool mapping

  std::vector<RUDecodeData> mRUDecodeVec;       // set of active RUs
  std::array<int, Mapping::getNRUs()> mRUEntry; // entry of the RU with given SW ID in the mRUDecodeVec
  uint16_t mCurRUDecodeID = NORUDECODED;        // index of currently processed RUDecode container
  Mapping mMAP;                                 // chip mapping
  int mVerbosity = 0;

  // statistics
  o2::itsmft::ROFRecord::ROFtype mROFCounter = 0; // RSTODO is this needed? eliminate from ROFRecord ?
  uint32_t mNChipsFiredROF = 0;                   // counter within the ROF
  uint32_t mNPixelsFiredROF = 0;                  // counter within the ROF
  size_t mNChipsFired = 0;                        // global counter
  size_t mNPixelsFired = 0;                       // global counter
  bool mDecodeNextAuto = true;                    // try to decode next trigger when getNextChipData does not see any decoded data
  TStopwatch mTimerTFStart;
  TStopwatch mTimerDecode;
  TStopwatch mTimeFetchData;

  ClassDefOverride(RawPixelDecoder, 1);
};

///______________________________________________________________
/// Fill decoded digits to global vector
template <class Mapping>
template <class DigitContainer, class ROFContainer>
int RawPixelDecoder<Mapping>::fillDecodedDigits(DigitContainer& digits, ROFContainer& rofs)
{
  if (mInteractionRecord.isDummy()) {
    return 0; // nothing was decoded
  }
  mTimeFetchData.Start(false);
  int ref = digits.size();
  for (unsigned int iru = 0; iru < mRUDecodeVec.size(); iru++) {
    for (int ic = 0; ic < mRUDecodeVec[iru].nChipsFired; ic++) {
      const auto& chip = mRUDecodeVec[iru].chipsData[ic];
      for (const auto& hit : mRUDecodeVec[iru].chipsData[ic].getData()) {
        digits.emplace_back(chip.getChipID(), hit.getRow(), hit.getCol());
      }
    }
  }
  int nFilled = digits.size() - ref;
  rofs.emplace_back(mInteractionRecord, mROFCounter, ref, nFilled);
  mTimeFetchData.Stop();
  return nFilled;
}

using RawDecoderITS = RawPixelDecoder<ChipMappingITS>;
using RawDecoderMFT = RawPixelDecoder<ChipMappingMFT>;

} // namespace itsmft
} // namespace o2

#endif /* ALICEO2_ITSMFT_RAWPIXELDECODER_H */