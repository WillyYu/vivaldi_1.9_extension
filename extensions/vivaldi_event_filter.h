// Copyright (c) 2016 Vivaldi Technologies AS. All rights reserved.

#ifndef EXTENSIONS_VIVALDI_EVENT_FILTER_H_
#define EXTENSIONS_VIVALDI_EVENT_FILTER_H_

#include <vector>
#include <string>

#include "base/macros.h"
#include "base/values.h"


namespace extensions {
class ScriptContext;
}

namespace vivaldi {

class VivaldiEventFilter {
public:
  VivaldiEventFilter(extensions::ScriptContext* context);
  ~VivaldiEventFilter();
  void AddGuestViewAndTabId(int instance_id, int tab_id);
  void RemoveGuestViewAndTabId(int instance_id, int tab_id);
  bool HasGuestView(int instance_id);
  int GetWindowId();
  bool HasTabIdInContext(int tab_id);

  bool Filter(const std::string event_name,
                            const base::ListValue* event_args,
                            const base::DictionaryValue* filtering_info);
private:
  extensions::ScriptContext* script_context_;
  bool is_vivaldi_app_;
  int window_id_;
  std::vector<int> guest_view_ids_;
  std::vector<int> tab_ids_;

  DISALLOW_COPY_AND_ASSIGN(VivaldiEventFilter);
};

}  // namespace vivaldi

#endif  // EXTENSIONS_VIVALDI_EVENT_FILTER_H_
