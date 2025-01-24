/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/public/ad_units/new_tab_page_ad/new_tab_page_ad_event_type_util.h"

#include "base/containers/fixed_flat_map.h"
#include "brave/components/brave_ads/core/mojom/brave_ads.mojom.h"

namespace brave_ads {

namespace {

constexpr auto kNewTabPageAdEventTypeMap = base::MakeFixedFlatMap<
    std::string,
    mojom::NewTabPageAdEventType>({
    // TODO(tmancey): @aseren the string represenations should match
    // `components/brave_ads/core/internal/account/confirmations/confirmation_type.cc`.
    {"served_impression", mojom::NewTabPageAdEventType::kServedImpression},
    {"viewed_impression", mojom::NewTabPageAdEventType::kViewedImpression},
    {"click", mojom::NewTabPageAdEventType::kClicked},
    {"media_play", mojom::NewTabPageAdEventType::kMediaPlay},
    {"media_25", mojom::NewTabPageAdEventType::kMedia25},
    {"media_100", brave_ads::mojom::NewTabPageAdEventType::kMedia100},
});

}  // namespace

std::optional<brave_ads::mojom::NewTabPageAdEventType>
ToMojomNewTabPageAdEventType(const std::string& event_type) {
  const auto iter = kNewTabPageAdEventTypeMap.find(event_type);
  if (iter == kNewTabPageAdEventTypeMap.cend()) {
    return std::nullopt;
  }

  return iter->second;
}


}  // namespace brave_ads
