// Copyright (c) 2016 Vivaldi Technologies AS. All rights reserved.

#include "extensions/vivaldi_event_filter.h"
#include "extensions/renderer/script_context.h"

#include "v8/include/v8.h"
#include "app/vivaldi_apptools.h"


namespace vivaldi {

VivaldiEventFilter::VivaldiEventFilter(
                    extensions::ScriptContext* context)
  : script_context_(context)
  , is_vivaldi_app_(IsVivaldiApp(script_context_->GetExtensionID()))
  , window_id_(-1) {

  // check this ScriptContext is a Vivaldi window
  // window_id_: -1 means no window_id exists
  //              0 means exists but not set
  if (is_vivaldi_app_
      && script_context_->url().ExtractFileName() == "browser.html") {
    window_id_ = 0;
  }
}

VivaldiEventFilter::~VivaldiEventFilter() {
}

void VivaldiEventFilter::AddGuestViewAndTabId(int instance_id, int tab_id) {
  guest_view_ids_.push_back(instance_id);
  tab_ids_.push_back(tab_id);
}

void VivaldiEventFilter::RemoveGuestViewAndTabId(int instance_id, int tab_id) {
  // remove instance_id
  std::vector<int>::iterator it = std::find_if(guest_view_ids_.begin(),
                                        guest_view_ids_.end(),
                                        [&instance_id](const int i) { return i == instance_id; });
  if (it != guest_view_ids_.end())
    guest_view_ids_.erase(it);

  if (tab_id > 0) {
    // remove tab_id
    it = std::find_if(tab_ids_.begin(),
                        tab_ids_.end(),
                        [&tab_id](const int i) { return i == tab_id; });
    if (it != tab_ids_.end())
      tab_ids_.erase(it);
  }
}

bool VivaldiEventFilter::HasGuestView(int instance_id) {
  std::vector<int>::iterator it = std::find_if(guest_view_ids_.begin()
                                        , guest_view_ids_.end()
                                        , [&instance_id](const int i) { return i == instance_id; });
  return it != guest_view_ids_.end();
}

int VivaldiEventFilter::GetWindowId() {
  if (window_id_ == 0) {
    v8::HandleScope handle_scope(script_context_->isolate());
    v8::Context::Scope context_scope(script_context_->v8_context());
    v8::Local<v8::Value> object =
            script_context_->v8_context()->Global()->Get(v8::String::NewFromUtf8(script_context_->isolate()
                                                        , "vivaldiWindowId"
                                                        , v8::NewStringType::kNormal)
                                                        .ToLocalChecked());
    if (!object.IsEmpty())
      window_id_ = object->Int32Value();
  }
  return window_id_;
}

bool VivaldiEventFilter::HasTabIdInContext(int tab_id){
  std::vector<int>::iterator it = std::find_if(tab_ids_.begin(),
                                                tab_ids_.end(),
                                                [&tab_id](const int i) { return i == tab_id; });
  return it != tab_ids_.end();
}

// Willy, dispatch event when vivaldiWindowId or tabId is matched.
//  Events webViewInternal.*, webViewPrivate.* and tabs.* will be filtered
bool VivaldiEventFilter::Filter(const std::string event_name,
                            const base::ListValue* event_args,
                            const base::DictionaryValue* filtering_info) {
  //if (!vivaldi::IsVivaldiApp(GetExtensionID()))
  if (!is_vivaldi_app_)
    return true;
  if (!event_name.compare(0, 16, "webViewInternal.")
        || !event_name.compare(0, 15, "webViewPrivate.")) {
    int inst_id = 0;
    filtering_info->GetInteger("instanceId", &inst_id);
    if (!HasGuestView(inst_id))
      return false;
  } else if (!event_name.compare(0, 5, "tabs.")
            || event_name == "tabsPrivate.onFaviconUpdated") {
    if (window_id_ == -1)
      return false;
    // tabs.onAttached, tabs.onDetached will not be handled
    const base::Value* v = nullptr;
    const int args_size = event_args->GetSize();
    int cur_window_id = GetWindowId();
    for (int i = 0; i < args_size; ++i) {

      if (!event_args->Get(i, &v)
          || v == nullptr
          || !v->is_dict())
        continue;

      const base::DictionaryValue* dict = nullptr;
      v->GetAsDictionary(&dict);
      const base::Value* id_value = nullptr;
      if (dict->Get("windowId", &id_value)
          && id_value->is_int()) {
        int window_id = 0;
        id_value->GetAsInteger(&window_id);
        if (window_id > 0 && window_id != cur_window_id)
          return false;
      } else if (dict->Get("tabId", &id_value)
                && id_value->is_int()) {
        int tab_id = 0;
        id_value->GetAsInteger(&tab_id);
        if (tab_id > 0 && !HasTabIdInContext(tab_id))
          return false;
      }
    }
  }
  return true;
}

}  // namespace vivaldi
