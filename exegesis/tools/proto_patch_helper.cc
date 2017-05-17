// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This program helps creating patches for a protobuf version of a pdf document
// created with the pdf2proto tool.
// Usage:
// bazel run -c opt \
// exegesis/tools:proto_patch_helper -- \
// --exegesis_proto_input_file=/path/to/sdm.pdf.pb \
// --exegesis_match_expression='SAL/SAR/SHL/SHR' \
// --exegesis_page_numbers=662

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "strings/string.h"

#include "gflags/gflags.h"

#include "exegesis/proto/pdf/pdf_document.pb.h"
#include "exegesis/util/pdf/xpdf_util.h"
#include "exegesis/util/proto_util.h"
#include "glog/logging.h"
#include "re2/re2.h"
#include "strings/str_split.h"
#include "util/gtl/map_util.h"

DEFINE_string(exegesis_proto_input_file, "",
              "Path to the binary proto representation of the PDF file.");
DEFINE_string(exegesis_match_expression, "",
              "The regular expression to match cells to patch.");
DEFINE_string(exegesis_page_numbers, "",
              "A list of page numbers to process, all pages if not set.");

namespace exegesis {
namespace pdf {
namespace {

bool ShouldProcessTextBlock(const PdfTextBlock& block) {
  if (FLAGS_exegesis_match_expression.empty()) return true;
  return RE2::PartialMatch(block.text(), FLAGS_exegesis_match_expression);
}

bool ShouldProcessPage(const std::unordered_set<size_t>& allowed_pages,
                       const PdfPage& page) {
  if (allowed_pages.empty()) return true;
  return ContainsKey(allowed_pages, page.number());
}

std::unordered_set<size_t> ParsePageNumbers() {
  std::unordered_set<size_t> pages;
  int page_number = 0;
  ::re2::

      StringPiece input(FLAGS_exegesis_page_numbers);
  while (RE2::Consume(&input, "(\\d+),?", &page_number)) {
    pages.insert(page_number);
  }
  CHECK(input.empty()) << "Can't parse page number '" << input << "'";
  return pages;
}

void Main() {
  CHECK(!FLAGS_exegesis_proto_input_file.empty())
      << "missing --exegesis_proto_input_file";
  CHECK(!FLAGS_exegesis_match_expression.empty())
      << "missing --exegesis_match_expression";

  const auto pdf_document =
      ReadBinaryProtoOrDie<PdfDocument>(FLAGS_exegesis_proto_input_file);

  // Prepare patches.
  const auto pages = ParsePageNumbers();
  std::map<size_t, std::vector<PdfPagePatch>> page_patches;
  for (const auto& page : pdf_document.pages()) {
    if (!ShouldProcessPage(pages, page)) continue;

    for (const auto& row : page.rows()) {
      for (const auto& block : row.blocks()) {
        if (ShouldProcessTextBlock(block)) {
          PdfPagePatch patch;
          patch.set_row(block.row());
          patch.set_col(block.col());
          patch.set_expected(block.text());
          patch.set_replacement(block.text());
          page_patches[page.number()].push_back(patch);
        }
      }
    }
  }

  // Gather patches per page.
  PdfDocumentsChanges documents_changes;
  auto* document_changes = documents_changes.add_documents();
  *document_changes->mutable_document_id() = pdf_document.document_id();
  for (const auto& page_patches_pair : page_patches) {
    auto* page_patches = document_changes->add_pages();
    page_patches->set_page_number(page_patches_pair.first);
    const auto& patches = page_patches_pair.second;
    std::copy(patches.begin(), patches.end(),
              google::protobuf::RepeatedFieldBackInserter(
                  page_patches->mutable_patches()));
  }

  // Display patches.
  documents_changes.PrintDebugString();
}

}  // namespace
}  // namespace pdf
}  // namespace exegesis

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  ::exegesis::pdf::Main();
  return 0;
}
