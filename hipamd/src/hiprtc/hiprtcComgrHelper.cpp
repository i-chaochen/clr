/*
Copyright (c) 2022 - Present Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "hiprtcComgrHelper.hpp"
#if defined(_WIN32)
#include <io.h>
#endif

#include "../amd_hsa_elf.hpp"

namespace hiprtc {

namespace helpers {

size_t constexpr strLiteralLength(char const* str) {
  return *str ? 1 + strLiteralLength(str + 1) : 0;
}

constexpr char const* CLANG_OFFLOAD_BUNDLER_MAGIC_STR = "__CLANG_OFFLOAD_BUNDLE__";
constexpr char const* OFFLOAD_KIND_HIP = "hip";
constexpr char const* OFFLOAD_KIND_HIPV4 = "hipv4";
constexpr char const* OFFLOAD_KIND_HCC = "hcc";
constexpr char const* AMDGCN_TARGET_TRIPLE = "amdgcn-amd-amdhsa-";

static constexpr size_t bundle_magic_string_size =
    strLiteralLength(CLANG_OFFLOAD_BUNDLER_MAGIC_STR);

struct __ClangOffloadBundleInfo {
  uint64_t offset;
  uint64_t size;
  uint64_t bundleEntryIdSize;
  const char bundleEntryId[1];
};

struct __ClangOffloadBundleHeader {
  const char magic[bundle_magic_string_size - 1];
  uint64_t numOfCodeObjects;
  __ClangOffloadBundleInfo desc[1];
};

uint64_t ElfSize(const void* emi) { return amd::Elf::getElfSize(emi); }

static bool getProcName(uint32_t EFlags, std::string& proc_name, bool& xnackSupported,
                        bool& sramEccSupported) {
  switch (EFlags & EF_AMDGPU_MACH) {
    case EF_AMDGPU_MACH_AMDGCN_GFX700:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx700";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX701:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx701";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX702:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx702";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX703:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx703";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX704:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx704";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX705:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx705";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX801:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx801";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX802:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx802";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX803:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx803";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX805:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx805";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX810:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx810";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX900:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx900";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX902:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx902";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX904:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx904";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX906:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx906";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX908:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx908";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX909:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx909";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX90A:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx90a";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX90C:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx90c";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX940:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx940";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX941:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx941";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX942:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx942";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX950:
      xnackSupported = true;
      sramEccSupported = true;
      proc_name = "gfx950";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1010:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx1010";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1011:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx1011";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1012:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx1012";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1013:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx1013";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1030:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1030";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1031:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1031";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1032:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1032";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1033:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1033";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1034:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1034";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1035:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1035";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1036:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1036";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1100:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1100";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1101:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1101";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1102:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1102";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1103:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1103";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1150:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1150";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1151:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1151";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1200:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1200";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX1201:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx1201";
    case EF_AMDGPU_MACH_AMDGCN_GFX9_GENERIC:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx9-generic";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX10_1_GENERIC:
      xnackSupported = true;
      sramEccSupported = false;
      proc_name = "gfx10-1-generic";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX10_3_GENERIC:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx10-3-generic";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX11_GENERIC:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx11-generic";
      break;
    case EF_AMDGPU_MACH_AMDGCN_GFX12_GENERIC:
      xnackSupported = false;
      sramEccSupported = false;
      proc_name = "gfx12-generic";
      break;
    default:
      return false;
  }
  return true;
}

static bool getTripleTargetIDFromCodeObject(const void* code_object, std::string& target_id) {
  if (!code_object) return false;
  const Elf64_Ehdr* ehdr = reinterpret_cast<const Elf64_Ehdr*>(code_object);
  if (ehdr->e_machine != EM_AMDGPU) return false;
  if (ehdr->e_ident[EI_OSABI] != ELFOSABI_AMDGPU_HSA) return false;

  bool isXnackSupported{false}, isSramEccSupported{false};

  std::string proc_name;
  if (!getProcName(ehdr->e_flags, proc_name, isXnackSupported, isSramEccSupported)) return false;
  target_id = std::string(AMDGCN_TARGET_TRIPLE) + '-' + proc_name;

  switch (ehdr->e_ident[EI_ABIVERSION]) {
    case ELFABIVERSION_AMDGPU_HSA_V2: {
      LogPrintfInfo("[Code Object V2, target id:%s]", target_id.c_str());
      return false;
    }

    case ELFABIVERSION_AMDGPU_HSA_V3: {
      LogPrintfInfo("[Code Object V3, target id:%s]", target_id.c_str());
      if (isSramEccSupported) {
        if (ehdr->e_flags & EF_AMDGPU_FEATURE_SRAMECC_V3)
          target_id += ":sramecc+";
        else
          target_id += ":sramecc-";
      }
      if (isXnackSupported) {
        if (ehdr->e_flags & EF_AMDGPU_FEATURE_XNACK_V3)
          target_id += ":xnack+";
        else
          target_id += ":xnack-";
      }
      break;
    }

    case ELFABIVERSION_AMDGPU_HSA_V4:
    case ELFABIVERSION_AMDGPU_HSA_V5:
    case ELFABIVERSION_AMDGPU_HSA_V6: {
      if (ehdr->e_ident[EI_ABIVERSION] & ELFABIVERSION_AMDGPU_HSA_V4) {
        LogPrintfInfo("[Code Object V4, target id:%s]", target_id.c_str());
      } else if (ehdr->e_ident[EI_ABIVERSION] & ELFABIVERSION_AMDGPU_HSA_V5) {
        LogPrintfInfo("[Code Object V5, target id:%s]", target_id.c_str());
      } else if (ehdr->e_ident[EI_ABIVERSION] & ELFABIVERSION_AMDGPU_HSA_V6) {
        LogPrintfInfo("[Code Object V6, target id:%s]", target_id.c_str());
      }

      unsigned co_sram_value = (ehdr->e_flags) & EF_AMDGPU_FEATURE_SRAMECC_V4;
      if (co_sram_value == EF_AMDGPU_FEATURE_SRAMECC_OFF_V4)
        target_id += ":sramecc-";
      else if (co_sram_value == EF_AMDGPU_FEATURE_SRAMECC_ON_V4)
        target_id += ":sramecc+";

      unsigned co_xnack_value = (ehdr->e_flags) & EF_AMDGPU_FEATURE_XNACK_V4;
      if (co_xnack_value == EF_AMDGPU_FEATURE_XNACK_OFF_V4)
        target_id += ":xnack-";
      else if (co_xnack_value == EF_AMDGPU_FEATURE_XNACK_ON_V4)
        target_id += ":xnack+";
      break;
    }

    default: {
      return false;
    }
  }
  return true;
}

// Consumes the string 'consume_' from the starting of the given input
// eg: input = amdgcn-amd-amdhsa--gfx908 and consume_ is amdgcn-amd-amdhsa--
// input will become gfx908.
static bool consume(std::string& input, std::string consume_) {
  if (input.substr(0, consume_.size()) != consume_) {
    return false;
  }
  input = input.substr(consume_.size());
  return true;
}

// Is agent target compatible with generic code object target?
static bool isCompatibleWithGenericTarget(std::string& coTarget, std::string& agentTarget) {
  // The map is subject to change per removing policy
  static std::map<std::string, std::string> genericTargetMap{
      // "gfx9-generic"
      {"gfx900", "gfx9-generic"},
      {"gfx902", "gfx9-generic"},
      {"gfx904", "gfx9-generic"},
      {"gfx906", "gfx9-generic"},
      {"gfx909", "gfx9-generic"},
      {"gfx90c", "gfx9-generic"},
      // "gfx10-1-generic"
      {"gfx1010", "gfx10-1-generic"},
      {"gfx1011", "gfx10-1-generic"},
      {"gfx1012", "gfx10-1-generic"},
      {"gfx1013", "gfx10-1-generic"},
      // "gfx10-3-generic"
      {"gfx1030", "gfx10-3-generic"},
      {"gfx1031", "gfx10-3-generic"},
      {"gfx1032", "gfx10-3-generic"},
      {"gfx1033", "gfx10-3-generic"},
      {"gfx1034", "gfx10-3-generic"},
      {"gfx1035", "gfx10-3-generic"},
      {"gfx1036", "gfx10-3-generic"},
      // "gfx11-generic"
      {"gfx1100", "gfx11-generic"},
      {"gfx1101", "gfx11-generic"},
      {"gfx1102", "gfx11-generic"},
      {"gfx1103", "gfx11-generic"},
      {"gfx1150", "gfx11-generic"},
      {"gfx1151", "gfx11-generic"},
  };
  auto search = genericTargetMap.find(agentTarget);
  return search != genericTargetMap.end() && coTarget == search->second;
}

// Trim String till character, will be used to get gpuname
// example: input is gfx908:sram-ecc+ and trim char is :
// input will become sram-ecc+.
static std::string trimName(std::string& input, char trim) {
  auto pos_ = input.find(trim);
  auto res = input;
  if (pos_ == std::string::npos) {
    input = "";
  } else {
    res = input.substr(0, pos_);
    input = input.substr(pos_);
  }
  return res;
}

static char getFeatureValue(std::string& input, std::string feature) {
  char res = ' ';
  if (consume(input, std::move(feature))) {
    res = input[0];
    input = input.substr(1);
  }
  return res;
}

static bool getTargetIDValue(std::string& input, std::string& processor, char& sramecc_value,
                             char& xnack_value) {
  processor = trimName(input, ':');
  sramecc_value = getFeatureValue(input, std::string(":sramecc"));
  if (sramecc_value != ' ' && sramecc_value != '+' && sramecc_value != '-') return false;
  xnack_value = getFeatureValue(input, std::string(":xnack"));
  if (xnack_value != ' ' && xnack_value != '+' && xnack_value != '-') return false;
  return true;
}

static bool getTripleTargetID(std::string bundled_co_entry_id, const void* code_object,
                       std::string& co_triple_target_id) {
  std::string offload_kind = trimName(bundled_co_entry_id, '-');
  if (offload_kind != OFFLOAD_KIND_HIPV4 && offload_kind != OFFLOAD_KIND_HIP &&
      offload_kind != OFFLOAD_KIND_HCC)
    return false;

  if (offload_kind != OFFLOAD_KIND_HIPV4)
    return getTripleTargetIDFromCodeObject(code_object, co_triple_target_id);

  // For code object V4 onwards the bundled code object entry ID correctly
  // specifies the target triple.
  co_triple_target_id = bundled_co_entry_id.substr(1);
  return true;
}

bool isCodeObjectCompatibleWithDevice(std::string co_triple_target_id,
         std::string agent_triple_target_id, unsigned& genericVersion) {
  // Primitive Check
  if (co_triple_target_id == agent_triple_target_id) return true;

  // Parse code object triple target id
  if (!consume(co_triple_target_id,
               std::string(OFFLOAD_KIND_HIP) + "-" + std::string(AMDGCN_TARGET_TRIPLE))) {
    return false;
  }

  std::string co_processor;
  char co_sram_ecc, co_xnack;
  if (!getTargetIDValue(co_triple_target_id, co_processor, co_sram_ecc, co_xnack)) {
    return false;
  }

  if (!co_triple_target_id.empty()) return false;

  // Parse agent isa triple target id
  if (!consume(agent_triple_target_id, std::string(AMDGCN_TARGET_TRIPLE) + '-')) {
    return false;
  }

  std::string agent_isa_processor;
  char isa_sram_ecc, isa_xnack;
  if (!getTargetIDValue(agent_triple_target_id, agent_isa_processor, isa_sram_ecc, isa_xnack)) {
    return false;
  }

  if (!agent_triple_target_id.empty()) return false;

  // Check for compatibility
  if (genericVersion >= EF_AMDGPU_GENERIC_VERSION_MIN) {
    // co_processor is generic target
    if (!isCompatibleWithGenericTarget(co_processor, agent_isa_processor))
    return false;
  } else if (agent_isa_processor != co_processor) {
    return false;
  }

  if (co_sram_ecc != ' ') {
    if (co_sram_ecc != isa_sram_ecc) return false;
  }
  if (co_xnack != ' ') {
    if (co_xnack != isa_xnack) return false;
  }

  return true;
}

static inline unsigned int getGenericVersion(const void* image) {
  const Elf64_Ehdr* ehdr = reinterpret_cast<const Elf64_Ehdr*>(image);
  return ehdr->e_ident[EI_ABIVERSION] == ELFABIVERSION_AMDGPU_HSA_V6
      ? ((ehdr->e_flags & EF_AMDGPU_GENERIC_VERSION) >> EF_AMDGPU_GENERIC_VERSION_OFFSET)
      : 0;
}

static inline bool isGenericTarget(const void* image) {
  return getGenericVersion(image) >= EF_AMDGPU_GENERIC_VERSION_MIN;
}

bool UnbundleBitCode(const std::vector<char>& bundled_llvm_bitcode, const std::string& isa,
                     size_t& co_offset, size_t& co_size) {
  std::string magic(bundled_llvm_bitcode.begin(),
                    bundled_llvm_bitcode.begin() + bundle_magic_string_size);
  if (magic.compare(CLANG_OFFLOAD_BUNDLER_MAGIC_STR)) {
    // Handle case where the whole file is unbundled
    return true;
  }

  std::string bundled_llvm_bitcode_s(bundled_llvm_bitcode.begin(),
                                     bundled_llvm_bitcode.begin() + bundled_llvm_bitcode.size());
  const void* data = reinterpret_cast<const void*>(bundled_llvm_bitcode_s.c_str());
  const auto obheader = reinterpret_cast<const __ClangOffloadBundleHeader*>(data);
  const auto* desc = &obheader->desc[0];
  for (uint64_t idx = 0; idx < obheader->numOfCodeObjects; ++idx,
                desc = reinterpret_cast<const __ClangOffloadBundleInfo*>(
                    reinterpret_cast<uintptr_t>(&desc->bundleEntryId[0]) +
                    desc->bundleEntryIdSize)) {
    const void* image =
        reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(obheader) + desc->offset);
    const size_t image_size = desc->size;
    std::string bundleEntryId{desc->bundleEntryId, desc->bundleEntryIdSize};

    // Need call getTripleTargetID(...).
    // Check if the device id and code object id are compatible
    unsigned genericVersion = getGenericVersion(image);
    if (isCodeObjectCompatibleWithDevice(bundleEntryId, isa, genericVersion)) {
      co_offset = (reinterpret_cast<uintptr_t>(image) - reinterpret_cast<uintptr_t>(data));
      co_size = image_size;
      break;
    }
  }
  return true;
}

bool addCodeObjData(amd_comgr_data_set_t& input, const std::vector<char>& source,
                    const std::string& name, const amd_comgr_data_kind_t type) {
  amd_comgr_data_t data;

  if (auto res = amd::Comgr::create_data(type, &data); res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  if (auto res = amd::Comgr::set_data(data, source.size(), source.data());
      res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::release_data(data);
    return false;
  }

  if (auto res = amd::Comgr::set_data_name(data, name.c_str()); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::release_data(data);
    return false;
  }

  if (auto res = amd::Comgr::data_set_add(input, data); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::release_data(data);
    return false;
  }
  amd::Comgr::release_data(data);  // Release from our end after setting the input

  return true;
}

bool extractBuildLog(amd_comgr_data_set_t dataSet, std::string& buildLog) {
  size_t count;
  if (auto res = amd::Comgr::action_data_count(dataSet, AMD_COMGR_DATA_KIND_LOG, &count);
      res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  std::vector<char> log;
  if (count > 0) {
    if (!extractByteCodeBinary(dataSet, AMD_COMGR_DATA_KIND_LOG, log)) return false;
    buildLog.insert(buildLog.end(), log.data(), log.data() + log.size());
  }
  return true;
}

bool extractByteCodeBinary(const amd_comgr_data_set_t inDataSet,
                           const amd_comgr_data_kind_t dataKind, std::vector<char>& bin) {
  amd_comgr_data_t binaryData;

  if (auto res = amd::Comgr::action_data_get_data(inDataSet, dataKind, 0, &binaryData);
      res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  size_t binarySize = 0;
  if (auto res = amd::Comgr::get_data(binaryData, &binarySize, NULL);
      res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::release_data(binaryData);
    return false;
  }

  size_t bufSize = (dataKind == AMD_COMGR_DATA_KIND_LOG) ? binarySize + 1 : binarySize;

  char* binary = new char[bufSize];
  if (binary == nullptr) {
    amd::Comgr::release_data(binaryData);
    return false;
  }


  if (auto res = amd::Comgr::get_data(binaryData, &binarySize, binary);
      res != AMD_COMGR_STATUS_SUCCESS) {
    delete[] binary;
    amd::Comgr::release_data(binaryData);
    return false;
  }

  if (dataKind == AMD_COMGR_DATA_KIND_LOG) {
    binary[binarySize] = '\0';
  }

  amd::Comgr::release_data(binaryData);

  std::vector<char> temp_bin;
  temp_bin.assign(binary, binary + binarySize);
  bin = temp_bin;
  delete[] binary;

  return true;
}

bool createAction(amd_comgr_action_info_t& action, std::vector<std::string>& options,
                  const std::string& isa, const amd_comgr_language_t lang) {
  if (auto res = amd::Comgr::create_action_info(&action); res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  if (lang != AMD_COMGR_LANGUAGE_NONE) {
    if (auto res = amd::Comgr::action_info_set_language(action, lang);
        res != AMD_COMGR_STATUS_SUCCESS) {
      amd::Comgr::destroy_action_info(action);
      return false;
    }
  }

  if (auto res = amd::Comgr::action_info_set_isa_name(action, isa.c_str());
      res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return false;
  }

  std::vector<const char*> optionsArgv;
  optionsArgv.reserve(options.size());
  for (auto& option : options) {
    optionsArgv.push_back(option.c_str());
  }

  if (auto res =
          amd::Comgr::action_info_set_option_list(action, optionsArgv.data(), optionsArgv.size());
      res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return res;
  }

  if (auto res = amd::Comgr::action_info_set_logging(action, true);
      res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return res;
  }

  return AMD_COMGR_STATUS_SUCCESS;
}

bool compileToExecutable(const amd_comgr_data_set_t compileInputs, const std::string& isa,
                         std::vector<std::string>& compileOptions,
                         std::vector<std::string>& linkOptions, std::string& buildLog,
                         std::vector<char>& exe) {
  amd_comgr_language_t lang = AMD_COMGR_LANGUAGE_HIP;
  amd_comgr_action_info_t action;
  amd_comgr_data_set_t reloc;
  amd_comgr_data_set_t output;
  amd_comgr_data_set_t input = compileInputs;

  if (auto res = createAction(action, compileOptions, isa, lang); res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  if (auto res = amd::Comgr::create_data_set(&reloc); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return false;
  }

  if (auto res = amd::Comgr::create_data_set(&output); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(reloc);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_COMPILE_SOURCE_TO_RELOCATABLE, action,
                                       input, reloc);
      res != AMD_COMGR_STATUS_SUCCESS) {
    extractBuildLog(reloc, buildLog);
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(reloc);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  if (!extractBuildLog(reloc, buildLog)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(reloc);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  amd::Comgr::destroy_action_info(action);
  if (auto res = createAction(action, linkOptions, isa, lang); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(reloc);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_LINK_RELOCATABLE_TO_EXECUTABLE, action,
                                       reloc, output);
      res != AMD_COMGR_STATUS_SUCCESS) {
    extractBuildLog(output, buildLog);
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    amd::Comgr::destroy_data_set(reloc);
    return false;
  }

  if (!extractBuildLog(output, buildLog)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    amd::Comgr::destroy_data_set(reloc);
    return false;
  }

  if (!extractByteCodeBinary(output, AMD_COMGR_DATA_KIND_EXECUTABLE, exe)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    amd::Comgr::destroy_data_set(reloc);
    return false;
  }

  // Clean up
  amd::Comgr::destroy_action_info(action);
  amd::Comgr::destroy_data_set(output);
  amd::Comgr::destroy_data_set(reloc);
  return true;
}

bool compileToBitCode(const amd_comgr_data_set_t compileInputs, const std::string& isa,
                      std::vector<std::string>& compileOptions, std::string& buildLog,
                      std::vector<char>& LLVMBitcode) {
  amd_comgr_language_t lang = AMD_COMGR_LANGUAGE_HIP;
  amd_comgr_action_info_t action;
  amd_comgr_data_set_t output;
  amd_comgr_data_set_t input = compileInputs;

  if (auto res = createAction(action, compileOptions, isa, lang); res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  if (auto res = amd::Comgr::create_data_set(&output); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_COMPILE_SOURCE_WITH_DEVICE_LIBS_TO_BC,
                                       action, input, output);
      res != AMD_COMGR_STATUS_SUCCESS) {
    extractBuildLog(output, buildLog);
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  if (!extractBuildLog(output, buildLog)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  if (!extractByteCodeBinary(output, AMD_COMGR_DATA_KIND_BC, LLVMBitcode)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  // Clean up
  amd::Comgr::destroy_action_info(action);
  amd::Comgr::destroy_data_set(output);
  return true;
}

bool linkLLVMBitcode(const amd_comgr_data_set_t linkInputs, const std::string& isa,
                     std::vector<std::string>& linkOptions, std::string& buildLog,
                     std::vector<char>& LinkedLLVMBitcode) {
  amd_comgr_language_t lang = AMD_COMGR_LANGUAGE_HIP;
  amd_comgr_action_info_t action;

  if (auto res = createAction(action, linkOptions, isa, AMD_COMGR_LANGUAGE_HIP);
      res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  amd_comgr_data_set_t output;
  if (auto res = amd::Comgr::create_data_set(&output); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_LINK_BC_TO_BC, action, linkInputs, output);
      res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  if (!extractBuildLog(output, buildLog)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  if (!extractByteCodeBinary(output, AMD_COMGR_DATA_KIND_BC, LinkedLLVMBitcode)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    return false;
  }

  amd::Comgr::destroy_action_info(action);
  amd::Comgr::destroy_data_set(output);
  return true;
}

bool createExecutable(const amd_comgr_data_set_t linkInputs, const std::string& isa,
                      std::vector<std::string>& exeOptions, std::string& buildLog,
                      std::vector<char>& executable) {
  amd_comgr_action_info_t action;

  if (auto res = createAction(action, exeOptions, isa); res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  amd_comgr_data_set_t relocatableData;
  if (auto res = amd::Comgr::create_data_set(&relocatableData); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_CODEGEN_BC_TO_RELOCATABLE, action,
                                       linkInputs, relocatableData);
      res != AMD_COMGR_STATUS_SUCCESS) {
    extractBuildLog(relocatableData, buildLog);
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }

  if (!extractBuildLog(relocatableData, buildLog)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }


  amd::Comgr::destroy_action_info(action);
  std::vector<std::string> emptyOpt;
  if (auto res = createAction(action, emptyOpt, isa); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }

  amd_comgr_data_set_t output;
  if (auto res = amd::Comgr::create_data_set(&output); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_LINK_RELOCATABLE_TO_EXECUTABLE, action,
                                       relocatableData, output);
      res != AMD_COMGR_STATUS_SUCCESS) {
    extractBuildLog(output, buildLog);
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }

  if (!extractBuildLog(output, buildLog)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }

  if (!extractByteCodeBinary(output, AMD_COMGR_DATA_KIND_EXECUTABLE, executable)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(output);
    amd::Comgr::destroy_data_set(relocatableData);
    return false;
  }

  amd::Comgr::destroy_action_info(action);
  amd::Comgr::destroy_data_set(output);
  amd::Comgr::destroy_data_set(relocatableData);

  return true;
}

void GenerateUniqueFileName(std::string& name) {
#if !defined(_WIN32)
  char* name_template = const_cast<char*>(name.c_str());
  int temp_fd = mkstemp(name_template);
#else
  char* name_template = new char[name.length() + 1];
  strcpy_s(name_template, name.length() + 1, name.data());
  int sizeinchars = strnlen(name_template, 20) + 1;
  _mktemp_s(name_template, sizeinchars);
#endif
  name = name_template;
#if !defined(_WIN32)
  unlink(name_template);
  close(temp_fd);
#endif
}

bool dumpIsaFromBC(const amd_comgr_data_set_t isaInputs, const std::string& isa,
                   std::vector<std::string>& exeOptions, std::string name, std::string& buildLog) {
  amd_comgr_action_info_t action;

  if (auto res = createAction(action, exeOptions, isa); res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  amd_comgr_data_set_t isaData;
  if (auto res = amd::Comgr::create_data_set(&isaData); res != AMD_COMGR_STATUS_SUCCESS) {
    amd::Comgr::destroy_action_info(action);
    return false;
  }

  if (auto res = amd::Comgr::do_action(AMD_COMGR_ACTION_CODEGEN_BC_TO_ASSEMBLY, action, isaInputs,
                                       isaData);
      res != AMD_COMGR_STATUS_SUCCESS) {
    extractBuildLog(isaData, buildLog);
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(isaData);
    return false;
  }

  std::vector<char> isaOutput;
  if (!extractByteCodeBinary(isaData, AMD_COMGR_DATA_KIND_SOURCE, isaOutput)) {
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(isaData);
    return false;
  }

  if (name.size() == 0) {
    // Generate a unique name if the program name is not specified by the user
    name = std::string("hiprtcXXXXXX");
    GenerateUniqueFileName(name);
  }
  std::string isaName = isa;
#if defined(_WIN32)
  // Replace special charaters that are not supported by Windows FS.
  std::replace(isaName.begin(), isaName.end(), ':', '@');
#endif

  auto isaFileName = name + std::string("-hip-") + isaName + ".s";
  std::ofstream f(isaFileName.c_str(), std::ios::trunc | std::ios::binary);
  if (f.is_open()) {
    f.write(isaOutput.data(), isaOutput.size());
    f.close();
  } else {
    buildLog += "Warning: writing isa file failed.\n";
    amd::Comgr::destroy_action_info(action);
    amd::Comgr::destroy_data_set(isaData);
    return false;
  }
  amd::Comgr::destroy_action_info(action);
  amd::Comgr::destroy_data_set(isaData);
  return true;
}

bool demangleName(const std::string& mangledName, std::string& demangledName) {
  amd_comgr_data_t mangled_data;
  amd_comgr_data_t demangled_data;

  if (AMD_COMGR_STATUS_SUCCESS != amd::Comgr::create_data(AMD_COMGR_DATA_KIND_BYTES, &mangled_data))
    return false;

  if (AMD_COMGR_STATUS_SUCCESS !=
      amd::Comgr::set_data(mangled_data, mangledName.size(), mangledName.c_str())) {
    amd::Comgr::release_data(mangled_data);
    return false;
  }

  if (AMD_COMGR_STATUS_SUCCESS != amd::Comgr::demangle_symbol_name(mangled_data, &demangled_data)) {
    amd::Comgr::release_data(mangled_data);
    return false;
  }

  size_t demangled_size = 0;
  if (AMD_COMGR_STATUS_SUCCESS != amd::Comgr::get_data(demangled_data, &demangled_size, NULL)) {
    amd::Comgr::release_data(mangled_data);
    amd::Comgr::release_data(demangled_data);
    return false;
  }

  demangledName.resize(demangled_size);

  if (AMD_COMGR_STATUS_SUCCESS !=
      amd::Comgr::get_data(demangled_data, &demangled_size,
                           const_cast<char*>(demangledName.data()))) {
    amd::Comgr::release_data(mangled_data);
    amd::Comgr::release_data(demangled_data);
    return false;
  }

  amd::Comgr::release_data(mangled_data);
  amd::Comgr::release_data(demangled_data);
  return true;
}

std::string handleMangledName(std::string loweredName) {
  if (loweredName.empty()) {
    return loweredName;
  }

  if (loweredName.find(".kd") != std::string::npos) {
    return {};
  }

  if (loweredName.find("void ") == 0) {
    loweredName.erase(0, strlen("void "));
  }

  auto dx{loweredName.find_first_of("(<")};

  if (dx == std::string::npos) {
    return loweredName;
  }

  if (loweredName[dx] == '<') {
    uint32_t count = 1;
    do {
      ++dx;
      count += (loweredName[dx] == '<') ? 1 : ((loweredName[dx] == '>') ? -1 : 0);
    } while (count);

    loweredName.erase(++dx);
  } else {
    loweredName.erase(dx);
  }

  return loweredName;
}

bool fillMangledNames(std::vector<char>& dataVec, std::map<std::string, std::string>& mangledNames,
                      bool isBitcode) {
  amd_comgr_data_t dataObject;
  if (auto res = amd::Comgr::create_data(
          isBitcode ? AMD_COMGR_DATA_KIND_BC : AMD_COMGR_DATA_KIND_EXECUTABLE, &dataObject);
      res != AMD_COMGR_STATUS_SUCCESS) {
    return false;
  }

  if (auto res = amd::Comgr::set_data(dataObject, dataVec.size(), dataVec.data())) {
    amd::Comgr::release_data(dataObject);
    return false;
  }

  size_t Count;
  if (auto res = amd::Comgr::populate_name_expression_map(dataObject, &Count)) {
    amd::Comgr::release_data(dataObject);
    return false;
  }

  for (auto& it : mangledNames) {
    size_t Size;
    char* data = const_cast<char*>(it.first.data());

    if (auto res = amd::Comgr::map_name_expression_to_symbol_name(dataObject, &Size, data, NULL)) {
      amd::Comgr::release_data(dataObject);
      return false;
    }

    std::unique_ptr<char[]> mName(new char[Size]());
    if (auto res =
            amd::Comgr::map_name_expression_to_symbol_name(dataObject, &Size, data, mName.get())) {
      amd::Comgr::release_data(dataObject);
      return false;
    }

    it.second = std::string(mName.get());
  }

  amd::Comgr::release_data(dataObject);
  return true;
}

}  // namespace helpers
}  // namespace hiprtc
