// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

namespace nu {

namespace {

// User clicks the close button.
gboolean OnClose(GtkWidget *widget, GdkEvent *event, Window* window) {
  if (window->should_close.is_null() || window->should_close.Run())
    window->Close();

  // We are destroying the window ourselves, so prevent the default behavior.
  return TRUE;
}

// Force window to allocate size for content view.
void ForceSizeAllocation(GtkWindow* window, GtkWidget* view) {
  GdkRectangle rect = { 0, 0 };
  gtk_window_get_size(window, &rect.width, &rect.height);
  gtk_widget_size_allocate(view, &rect);
}

}  // namespace

Window::~Window() {
  if (window_)
    gtk_widget_destroy(GTK_WIDGET(window_));
}

void Window::PlatformInit(const Options& options) {
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_focus_on_map(window_, false);

  if (!options.bounds.IsEmpty()) {
    gtk_window_set_default_size(window_, options.bounds.width(),
                                         options.bounds.height());
    gtk_window_move(window_, options.bounds.x(), options.bounds.y());
  }

  g_signal_connect(window_, "delete-event", G_CALLBACK(OnClose), this);
}

void Window::Close() {
  if (!should_close.is_null() && !should_close.Run())
    return;

  on_close.Emit();
  gtk_widget_destroy(GTK_WIDGET(window_));

  window_ = nullptr;
}

void Window::PlatformSetContentView(Container* container) {
  GtkWidget* child = gtk_bin_get_child(GTK_BIN(window_));
  if (child)
    gtk_container_remove(GTK_CONTAINER(window_), child);
  gtk_container_add(GTK_CONTAINER(window_), container->view());

  ForceSizeAllocation(window_, container->view());
}

void Window::SetContentBounds(const Rect& bounds) {
  SetBounds(bounds);
}

Rect Window::GetContentBounds() const {
  return GetBounds();
}

void Window::SetBounds(const Rect& bounds) {
  gtk_window_move(window_, bounds.x(), bounds.y());
  gtk_window_resize(window_, bounds.width(), bounds.height());

  ForceSizeAllocation(window_, GetContentView()->view());
}

Rect Window::GetBounds() const {
  int x, y, width, height;
  gtk_window_get_position(window_, &x, &y);
  gtk_window_get_size(window_, &width, &height);
  return Rect(x, y, width, height);
}

void Window::SetVisible(bool visible) {
  gtk_widget_set_visible(GTK_WIDGET(window_), visible);
}

bool Window::IsVisible() const {
  return gtk_widget_get_visible(GTK_WIDGET(window_));
}

}  // namespace nu