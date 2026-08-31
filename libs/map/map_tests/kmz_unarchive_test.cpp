#include "testing/testing.hpp"

#include "map/bookmark.hpp"
#include "map/bookmark_helpers.hpp"

#include "platform/platform.hpp"

#include "coding/file_reader.hpp"
#include "coding/file_writer.hpp"
#include "coding/internal/file_data.hpp"
#include "coding/zip_creator.hpp"

#include "base/file_name_utils.hpp"
#include "base/scope_guard.hpp"

#include <string>

UNIT_TEST(KMZ_UnzipTest)
{
  /// @todo Should put somewhere in core? (like in BookmarkManager::PrepareToSaveBookmarks).
  TEST(Platform::MkDirChecked(GetBookmarksDirectory()), ());

  std::string const kmzFile = GetPlatform().TestsDataPathForFile("test_data/kml/test.kmz");
  auto const filePaths = GetKMLOrGPXFilesPathsToLoad(kmzFile);
  TEST_EQUAL(1, filePaths.size(), ());
  auto const & filePath = filePaths[0];
  TEST(filePath.ends_with("doc.kml"), (filePath));

  SCOPE_GUARD(fileGuard, std::bind(&base::DeleteFileX, filePath));

  auto const kmlData = LoadKmlFile(filePath, FileType::Kml);
  TEST(kmlData != nullptr, ());

  TEST_EQUAL(kmlData->m_bookmarksData.size(), 6, ("Category wrong number of bookmarks"));

  {
    Bookmark const bm(std::move(kmlData->m_bookmarksData[0]));
    TEST_EQUAL(kml::GetDefaultStr(bm.GetName()), ("Lahaina Breakwall"), ("KML wrong name!"));
    TEST_EQUAL(bm.GetColor(), kml::PredefinedColor::Red, ("KML wrong type!"));
    TEST_ALMOST_EQUAL_ULPS(bm.GetPivot().x, -156.6777046791284, ("KML wrong org x!"));
    TEST_ALMOST_EQUAL_ULPS(bm.GetPivot().y, 21.34256685860084, ("KML wrong org y!"));
    TEST_EQUAL(bm.GetScale(), 0, ("KML wrong scale!"));
  }
  {
    Bookmark const bm(std::move(kmlData->m_bookmarksData[1]));
    TEST_EQUAL(kml::GetDefaultStr(bm.GetName()), ("Seven Sacred Pools, Kipahulu"), ("KML wrong name!"));
    TEST_EQUAL(bm.GetColor(), kml::PredefinedColor::Red, ("KML wrong type!"));
    TEST_ALMOST_EQUAL_ULPS(bm.GetPivot().x, -156.0405130750025, ("KML wrong org x!"));
    TEST_ALMOST_EQUAL_ULPS(bm.GetPivot().y, 21.12480639056074, ("KML wrong org y!"));
    TEST_EQUAL(bm.GetScale(), 0, ("KML wrong scale!"));
  }
}

namespace
{
void WriteFile(std::string const & path, std::string const & content)
{
  FileWriter writer(path);
  writer.Write(content.data(), content.size());
}
}  // namespace

UNIT_TEST(KMZ_ExtractPhotosTest)
{
  TEST(Platform::MkDirChecked(GetBookmarksDirectory()), ());

  auto const & writableDir = GetPlatform().WritableDir();
  std::string const kmlPath = base::JoinPath(writableDir, "om_kmz_photos_test_doc.kml");
  std::string const imgPath = base::JoinPath(writableDir, "om_kmz_photos_test_photo.jpg");
  std::string const kmzPath = base::JoinPath(writableDir, "om_kmz_photos_test.kmz");

  std::string const kmlContent =
      R"(<?xml version="1.0" encoding="UTF-8"?><kml xmlns="http://www.opengis.net/kml/2.2"><Document></Document></kml>)";
  std::string const imgContent = "\xFF\xD8\xFF\xE0 not a real jpeg, just bytes";
  std::string const photoInZip = "photos/1ea01b46-f885-4f68-b636/1210009_739326.jpg";

  WriteFile(kmlPath, kmlContent);
  WriteFile(imgPath, imgContent);

  std::string const extractedPhoto =
      base::JoinPath(GetBookmarkPhotosDirectory(), "photos", "1ea01b46-f885-4f68-b636", "1210009_739326.jpg");
  std::string const extractedKml = base::JoinPath(GetBookmarkPhotosDirectory(), "doc.kml");

  SCOPE_GUARD(cleanup, [&]()
  {
    base::DeleteFileX(kmlPath);
    base::DeleteFileX(imgPath);
    base::DeleteFileX(kmzPath);
    base::DeleteFileX(extractedPhoto);
    base::DeleteFileX(extractedKml);
  });

  TEST(CreateZipFromFiles({kmlPath, imgPath}, {"doc.kml", photoInZip}, kmzPath), ());

  ExtractBookmarkAssetsFromKmz(kmzPath);

  TEST(Platform::IsFileExistsByFullPath(extractedPhoto), (extractedPhoto));
  std::string extractedContent;
  FileReader(extractedPhoto).ReadAsString(extractedContent);
  TEST_EQUAL(extractedContent, imgContent, ());
  TEST(!Platform::IsFileExistsByFullPath(extractedKml), (extractedKml));
}

UNIT_TEST(Multi_KML_KMZ_UnzipTest)
{
  TEST(Platform::MkDirChecked(GetBookmarksDirectory()), ());

  std::string const kmzFile = GetPlatform().TestsDataPathForFile("test_data/kml/BACRNKMZ.kmz");
  auto const filePaths = GetKMLOrGPXFilesPathsToLoad(kmzFile);
  SCOPE_GUARD(filesGuard, [&filePaths]()
  {
    for (auto const & path : filePaths)
      base::DeleteFileX(path);
  });

  base::StringIL expectedFileNames = {
      "BACRNKMZfilesCampgrounds 26may2022 green and tree icon",
      "BACRNKMZfilesIndoor Accommodations 26may2022 placemark purple and bed icon",
      "BACRNKMZfilesRoute 1 Canada - West-East Daily Segments",
      "BACRNKMZfilesRoute 2 Canada - West-East Daily Segments",
      "BACRNKMZfilesRoute Connector Canada - West-East Daily Segments",
      "BACRNKMZdoc",
  };
  TEST_EQUAL(expectedFileNames.size(), filePaths.size(), ());

  for (auto const & filePath : filePaths)
  {
    auto matched = false;
    for (auto const & expectedFileName : expectedFileNames)
    {
      matched = filePath.find(expectedFileName) != std::string::npos;
      if (matched)
        break;
    }
    TEST(matched, ("Unexpected file path: " + filePath));
  }
}
