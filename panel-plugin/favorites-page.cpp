/*
 * Copyright (C) 2013-2021 Graeme Gott <graeme@gottcode.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "favorites-page.h"

#include "applications-page.h"
#include "image-menu-item.h"
#include "launcher.h"
#include "launcher-view.h"
#include "settings.h"
#include "slot.h"
#include "window.h"

#include <algorithm>

#include <glib/gi18n-lib.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

FavoritesPage::FavoritesPage(Window* window) :
	Page(window, "document-open-favorites", _("Favourites"))
{
	// Prevent going over max
	if (wm_settings->favorites.size() > wm_settings->favorites_items_max)
	{
		wm_settings->favorites.resize(wm_settings->favorites_items_max);
	}
}

//-----------------------------------------------------------------------------

FavoritesPage::~FavoritesPage()
{
	unset_menu_items();
}

//-----------------------------------------------------------------------------

void FavoritesPage::add(Launcher* launcher)
{
	if (!wm_settings->favorites_items_max || !launcher)
	{
		return;
	}
	launcher->set_flag(Launcher::FavoritesFlag, true);

	std::string desktop_id = launcher->get_desktop_id();
	if (!wm_settings->favorites.empty())
	{
		auto i = std::find(wm_settings->favorites.begin(), wm_settings->favorites.end(), desktop_id);

		// Skip if already first launcher
		if (i == wm_settings->favorites.begin())
		{
			return;
		}
		// Move to front if already in list
		else if (i != wm_settings->favorites.end())
		{
			const int pos = std::distance(wm_settings->favorites.begin(), i);
			GtkTreeModel* model = get_view()->get_model();
			GtkTreeIter iter;
			gtk_tree_model_iter_nth_child(model, &iter, nullptr, pos);
			gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, nullptr);
			wm_settings->favorites.erase(pos);
			wm_settings->favorites.insert(0, desktop_id);
			return;
		}
	}

	// Prepend to list of items
	GtkListStore* store = GTK_LIST_STORE(get_view()->get_model());
	gtk_list_store_insert_with_values(
			store, nullptr, 0,
			LauncherView::COLUMN_ICON, launcher->get_icon(),
			LauncherView::COLUMN_TEXT, launcher->get_text(),
			LauncherView::COLUMN_TOOLTIP, launcher->get_tooltip(),
			LauncherView::COLUMN_LAUNCHER, launcher,
			-1);
	wm_settings->favorites.insert(0, desktop_id);

	// Prevent going over max
	enforce_item_count();
}

//-----------------------------------------------------------------------------

void FavoritesPage::enforce_item_count()
{
	if (wm_settings->favorites.size() <= wm_settings->favorites_items_max)
	{
		return;
	}

	GtkListStore* store = GTK_LIST_STORE(get_view()->get_model());
	for (int i = wm_settings->favorites.size() - 1, end = wm_settings->favorites_items_max; i >= end; --i)
	{
		Launcher* launcher = get_window()->get_applications()->find(wm_settings->favorites[i]);
		if (launcher)
		{
			launcher->set_flag(Launcher::FavoritesFlag, false);
		}

		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, nullptr, i))
		{
			gtk_list_store_remove(store, &iter);
		}
	}

	wm_settings->favorites.resize(wm_settings->favorites_items_max);
}

//-----------------------------------------------------------------------------

void FavoritesPage::flag_items(bool enabled)
{
	for (const auto& favorites : wm_settings->favorites)
	{
		Launcher* launcher = get_window()->get_applications()->find(favorites);
		if (launcher)
		{
			launcher->set_flag(Launcher::FavoritesFlag, enabled);
		}
	}
}

//-----------------------------------------------------------------------------

void FavoritesPage::set_menu_items()
{
	GtkTreeModel* model = get_window()->get_applications()->create_launcher_model(wm_settings->favorites);
	get_view()->set_model(model);
	g_object_unref(model);

	flag_items(true);
}

//-----------------------------------------------------------------------------

void FavoritesPage::unset_menu_items()
{
	// Clear treeview
	get_view()->unset_model();
}

//-----------------------------------------------------------------------------

void FavoritesPage::extend_context_menu(GtkWidget* menu)
{
	GtkWidget* menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = whiskermenu_image_menu_item_new("edit-clear", _("Clear Favorites"));
	connect(menuitem, "activate",
		[this](GtkMenuItem*)
		{
			flag_items(false);
			gtk_list_store_clear(GTK_LIST_STORE(get_view()->get_model()));
			wm_settings->favorites.clear();
		});
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

//-----------------------------------------------------------------------------
