<?php
/*
** Zabbix
** Copyright (C) 2001-2024 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/


/**
 * @var CView $this
 */
?>

<script type="text/javascript">

	const view = new class {
		init() {
			document.getElementById('user-form').addEventListener('submit', (e) => {
				if (!this._userFormSubmit()) {
					e.preventDefault();
				}
			});

			document.getElementById('messages_enabled').addEventListener('click', () => {
				this._updateForm();
			});

			this._updateForm();
		}

		_updateForm() {
			document
				.getElementById('messagingTab')
				.querySelectorAll('input:not([name="messages[enabled]"]),button')
				.forEach((elem) => {
					elem.toggleAttribute('disabled', !document.getElementById('messages_enabled').checked);
				});
		}

		_userFormSubmit() {
			document.querySelectorAll('#autologout, #refresh, #url').forEach((elem) => {
				elem.value = elem.value.trim();
			});

			const elem_current = document.getElementById('current_password');
			const elem_password1 = document.getElementById('password1');
			const elem_password2 = document.getElementById('password2');

			if (elem_current && elem_password1 && elem_password2) {
				const current_password = elem_current.value;
				const password1 = elem_password1.value;
				const password2 = elem_password2.value;

				if (password1 !== '' && password2 !== '' && current_password !== '') {
					const warning_msg = <?= json_encode(
						_('In case of successful password change user will be logged out of all active sessions. Continue?')
					) ?>;

					return confirm(warning_msg);
				}
			}

			return true;
		}
	}
</script>
