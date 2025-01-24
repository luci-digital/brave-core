/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import styled from 'styled-components'

export interface SponsoredRichMediaBackgroundInfo {
  url: string
  creativeInstanceId: string
  placementId: string
  targetUrl: string
}

interface StatusProps {
  sponsoredRichMediaBackgroundHasLoaded: boolean
}

interface Props extends StatusProps {
  sponsoredRichMediaBackgroundInfo?: SponsoredRichMediaBackgroundInfo | undefined
  onEventReported: (name: NewTab.SponsoredRichMediaEventType) => void
}

const iframeAllow = `
  accelerometer 'none';
  ambient-light-sensor 'none';
  camera 'none';
  display-capture 'none';
  document-domain 'none';
  fullscreen 'none';
  geolocation 'none';
  gyroscope 'none';
  magnetometer 'none';
  microphone 'none';
  midi 'none';
  payment 'none';
  publickey-credentials-get 'none';
  usb 'none'
`

const SponsoredRichMediaBackgroundIframe = styled('iframe') <StatusProps>`
  --bg-opacity: ${p => p.sponsoredRichMediaBackgroundHasLoaded ? 1 : 0};
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  border: none;
  z-index: 0;
`

/// We expect the event data to be of the following format:
/// {
///   type: 'richMediaEvent',
///   value: 'click'
/// }
function getEventType(event: MessageEvent): NewTab.SponsoredRichMediaEventType | undefined {
  if (!event.data || event.data.type !== 'richMediaEvent') {
    return undefined
  }

  if (event.data.value === 'click' || event.data.value === 'mediaPlay'
      || event.data.value === 'media25' || event.data.value === 'media100') {
    return event.data.value as NewTab.SponsoredRichMediaEventType
  }

  return undefined
}

export function SponsoredRichMediaBackground (props: Props) {
  const iframeRef = React.useRef<HTMLIFrameElement | null>(null)
  const { sponsoredRichMediaBackgroundInfo } = props

  if (!sponsoredRichMediaBackgroundInfo) {
    return null
  }

  React.useEffect(() => {
    const listener = (event: MessageEvent) => {
      if (!iframeRef.current) {
        return
      }

      const { contentWindow } = iframeRef.current
      if (!event.source || event.source !== contentWindow || !event.data) {
        return
      }

      const eventType = getEventType(event)
      if (!eventType) {
        return
      }

      props.onEventReported(eventType)
    }

    window.addEventListener('message', listener)
    return () => { window.removeEventListener('message', listener) }
  }, [props.onEventReported, sponsoredRichMediaBackgroundInfo])

  return (
    <SponsoredRichMediaBackgroundIframe
          ref={iframeRef}
          sponsoredRichMediaBackgroundHasLoaded={props.sponsoredRichMediaBackgroundHasLoaded}
          allow={iframeAllow.trim().replace(/\n/g, '')}
          src={sponsoredRichMediaBackgroundInfo.url}
          sandbox='allow-scripts allow-same-origin'>
    </SponsoredRichMediaBackgroundIframe>
  )
}
