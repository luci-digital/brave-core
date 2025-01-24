/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/ntp_background_images/browser/ntp_sponsored_rich_media_source.h"

#include <optional>
#include <utility>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/memory/ref_counted_memory.h"
#include "base/task/thread_pool.h"
#include "brave/components/constants/webui_url_constants.h"
#include "brave/components/ntp_background_images/browser/ntp_background_images_service.h"
#include "brave/components/ntp_background_images/browser/ntp_sponsored_images_data.h"
#include "brave/components/ntp_background_images/browser/view_counter_service.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/mime_util.h"
#include "url/gurl.h"

namespace ntp_background_images {

namespace {

std::optional<std::string> ReadFileToString(const base::FilePath& path) {
  std::string contents;
  if (!base::ReadFileToString(path, &contents)) {
    return std::optional<std::string>();
  }
  return contents;
}

}  // namespace

NTPSponsoredRichMediaSource::NTPSponsoredRichMediaSource(
    NTPBackgroundImagesService* service)
    : service_(service), weak_factory_(this) {}

NTPSponsoredRichMediaSource::~NTPSponsoredRichMediaSource() = default;

std::string NTPSponsoredRichMediaSource::GetSource() {
  return kRichMediaURL;
}

void NTPSponsoredRichMediaSource::StartDataRequest(
    const GURL& url,
    const content::WebContents::Getter& wc_getter,
    GotDataCallback callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  const std::string path = URLDataSource::URLToRequestPath(url);
  const std::optional<base::FilePath> file_path =
      MaybeGetFilePathForDataRequest(path);
  if (!file_path) {
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE, base::BindOnce(std::move(callback),
                                  scoped_refptr<base::RefCountedMemory>()));

    return;
  }

  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock()},
      base::BindOnce(&ReadFileToString, *file_path),
      base::BindOnce(&NTPSponsoredRichMediaSource::ReadFileCallback,
                     weak_factory_.GetWeakPtr(), std::move(callback)));
}

void NTPSponsoredRichMediaSource::ReadFileCallback(
    GotDataCallback callback,
    std::optional<std::string> input) {
  if (!input) {
    return std::move(callback).Run(scoped_refptr<base::RefCountedMemory>());
  }

  std::move(callback).Run(
      new base::RefCountedBytes(base::as_byte_span(*input)));
}

std::string NTPSponsoredRichMediaSource::GetMimeType(const GURL& url) {
  std::string mime_type;
  std::string file_path_extension =
      base::FilePath(url.path_piece()).Extension();
  if (!file_path_extension.empty()) {
    net::GetWellKnownMimeTypeFromExtension(file_path_extension.substr(1),
                                           &mime_type);
  }

  return mime_type;
}

bool NTPSponsoredRichMediaSource::AllowCaching() {
  return false;
}

std::string NTPSponsoredRichMediaSource::GetContentSecurityPolicy(
    network::mojom::CSPDirectiveName directive) {
  switch (directive) {
    case network::mojom::CSPDirectiveName::FrameAncestors:
      return std::string("frame-ancestors ") + kBraveNewTabPageURL + ";";
    case network::mojom::CSPDirectiveName::Sandbox:
      return std::string("sandbox allow-scripts;");
    case network::mojom::CSPDirectiveName::DefaultSrc:
      return std::string("default-src 'none';");
    case network::mojom::CSPDirectiveName::BaseURI:
      return std::string("base-uri 'none';");
    case network::mojom::CSPDirectiveName::FormAction:
      return std::string("form-action 'none';");
    case network::mojom::CSPDirectiveName::ScriptSrc:
      return std::string("script-src 'self' 'unsafe-inline';");
    case network::mojom::CSPDirectiveName::StyleSrc:
      return std::string("style-src 'self';");
    case network::mojom::CSPDirectiveName::ImgSrc:
      return std::string("img-src 'self';");
    case network::mojom::CSPDirectiveName::MediaSrc:
      return std::string("media-src 'self';");
    default:
      return content::URLDataSource::GetContentSecurityPolicy(directive);
  }
}

std::optional<base::FilePath>
NTPSponsoredRichMediaSource::MaybeGetFilePathForDataRequest(
    const std::string& path) {
  const NTPSponsoredImagesData* const images_data =
      service_->GetBrandedImagesData(false);
  if (!images_data) {
    return std::nullopt;
  }

  const std::string creative_file_path =
      base::FilePath::FromUTF8Unsafe(path).DirName().AsUTF8Unsafe();

  const base::FilePath base_name =
      base::FilePath::FromUTF8Unsafe(path).BaseName();

  for (const auto& campaign : images_data->campaigns) {
    for (const auto& creative : campaign.backgrounds) {
      if (creative_file_path == creative.creative_instance_id) {
        return creative.wallpaper_file.DirName().Append(base_name);
      }
    }
  }

  return std::nullopt;
}

}  // namespace ntp_background_images
