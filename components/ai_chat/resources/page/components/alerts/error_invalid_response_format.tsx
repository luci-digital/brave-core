/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import Alert from '@brave/leo/react/alert'
import { getLocale } from '$web-common/locale'
import styles from './alerts.module.scss'

export default function ErrorInvalidResponseFormat() {
  return (
    <div className={styles.alert}>
      <Alert type='error'>
        {getLocale('errorInvalidResponseFormat')}
      </Alert>
    </div>
  )
}
