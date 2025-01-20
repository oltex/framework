#pragma once
#include "../design-pattern/singleton.h"
#include "../data-strucutre/pair.h"
#include "../data-strucutre/unique_pointer.h"
#include <Pdh.h>
#pragma comment(lib,"pdh.lib")
#include <unordered_map>
#include <string>

#include <iostream>

namespace utility {
	class performance_monitor final : public design_pattern::singleton<performance_monitor> {
	private:
		friend class design_pattern::singleton<performance_monitor>;
		//using size_type = unsigned int;
	private:
		inline explicit performance_monitor(void) noexcept {
			PdhOpenQueryW(nullptr, NULL, &_qurey);
		};
		inline explicit performance_monitor(performance_monitor const& rhs) noexcept = delete;
		inline explicit performance_monitor(performance_monitor&& rhs) noexcept = delete;
		inline auto operator=(performance_monitor const& rhs) noexcept -> performance_monitor & = delete;
		inline auto operator=(performance_monitor&& rhs) noexcept -> performance_monitor & = delete;
		inline ~performance_monitor(void) noexcept {
			PdhCloseQuery(&_qurey);
		}
	public:
		inline void add_counter(std::wstring_view const key, std::wstring_view const object, std::wstring_view const item, std::wstring_view const instance) noexcept {
			size_t size = sizeof(wchar_t) * (object.size() + item.size() + instance.size() + 7);
			wchar_t* path = reinterpret_cast<wchar_t*>(malloc(size));
			swprintf_s(path, size / sizeof(wchar_t), L"\\%s(%s)\\%% %s", object.data(), instance.data(), item.data());
			auto& iter = _counter.emplace(std::piecewise_construct, std::forward_as_tuple(key.data()), std::forward_as_tuple()).first->second;
			PdhAddCounterW(_qurey, path, NULL, &iter);
			free(path);
		}
		inline void add_counter(std::wstring_view const key, std::wstring_view const object, std::wstring_view const item) noexcept {
			size_t size = object.size() + item.size() + 5;
			wchar_t* path = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * size));
			swprintf_s(path, size / sizeof(wchar_t), L"\\%s\\%% %s", object.data(), item.data());
			auto& iter = _counter.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple()).first->second;
			PdhAddCounterW(_qurey, path, NULL, &iter);
			free(path);
		}
		inline void add_counter(std::wstring_view const key, std::wstring_view const path) noexcept {
			auto& iter = _counter.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple()).first->second;
			PdhAddCounterW(_qurey, path.data(), NULL, &iter);
		}
		inline void remove_counter(std::wstring_view const key) noexcept {
			auto iter = _counter.find(key.data());
			PdhRemoveCounter(_counter.find(key.data())->second);
			_counter.erase(iter);
		}
		inline void collect_query_data(void) noexcept {
			PdhCollectQueryData(_qurey);
		}
		inline void collect_query_data_ex(void) noexcept {
			//PdhCollectQueryDataEx()
		}
		inline auto get_formatted_counter_value(std::wstring_view const key, unsigned long format) noexcept -> PDH_FMT_COUNTERVALUE {
			auto& iter = _counter.find(key.data())->second;
			PDH_FMT_COUNTERVALUE counter_value;
			PdhGetFormattedCounterValue(iter, format, nullptr, &counter_value);
			return counter_value;
		}
		inline auto get_formatted_counter_array(std::wstring_view const key, unsigned long format) noexcept -> data_structure::pair<unsigned long, data_structure::unique_pointer<PDH_FMT_COUNTERVALUE_ITEM_W[]>> {
			auto& iter = _counter.find(key.data())->second;

			unsigned long buffer_size = 0;
			unsigned long item_count;
			PDH_STATUS status1 = PdhGetFormattedCounterArrayW(iter, format, &buffer_size, &item_count, nullptr);
			PDH_FMT_COUNTERVALUE_ITEM_W* counter_value_item = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(malloc(buffer_size));
			PDH_STATUS status2 = PdhGetFormattedCounterArrayW(iter, format, &buffer_size, &item_count, counter_value_item);

			return data_structure::pair<unsigned long, data_structure::unique_pointer<PDH_FMT_COUNTERVALUE_ITEM_W[]>>(item_count, counter_value_item);
		}
	public:
		inline auto enum_object(std::wstring_view const filter = L"*") noexcept -> std::list<std::wstring> {
			unsigned long object_size = 0;
			PdhEnumObjectsW(nullptr, nullptr, nullptr, &object_size, PERF_DETAIL_WIZARD, TRUE);
			wchar_t* object_buffer = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * object_size));
			PdhEnumObjectsW(nullptr, nullptr, object_buffer, &object_size, PERF_DETAIL_WIZARD, FALSE);
			wchar_t const* object = object_buffer;

			std::list<std::wstring> result;
			if (L"*" == filter)
				for (wchar_t const* object = object_buffer; *object != false; object += wcslen(object) + 1)
					result.emplace_back(object);
			else
				for (wchar_t const* object = object_buffer; *object != false; object += wcslen(object) + 1) {
					std::wstring current(object);
					if (std::wstring::npos != current.find(filter))
						result.emplace_back(object);
				}
			free(object_buffer);

			result.sort();
			return result;
		}
		inline auto enum_item_instance(std::wstring_view const object) noexcept -> std::pair<std::list<std::wstring>, std::list<std::wstring>> {
			unsigned long item_size = 0;
			unsigned long instance_size = 0;
			PdhEnumObjectItemsW(nullptr, nullptr, object.data(), nullptr, &item_size, nullptr, &instance_size, PERF_DETAIL_WIZARD, 0);
			wchar_t* item_buffer = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * item_size));
			wchar_t* instance_buffer = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * instance_size));
			PdhEnumObjectItemsW(nullptr, nullptr, object.data(), item_buffer, &item_size, instance_buffer, &instance_size, PERF_DETAIL_WIZARD, 0);

			std::pair<std::list<std::wstring>, std::list<std::wstring>> result;
			for (wchar_t const* item = item_buffer; *item != false; item += wcslen(item) + 1)
				result.first.emplace_back(item);
			for (wchar_t const* instance = instance_buffer; *instance != false; instance += wcslen(instance) + 1)
				result.second.emplace_back(instance);

			free(item_buffer);
			free(instance_buffer);

			result.first.sort();
			result.second.sort();
			return result;
		}
		inline auto expand_counter_path(std::wstring_view const path) noexcept {
		}
	private:
		PDH_HQUERY _qurey;
		std::unordered_map<std::wstring, PDH_HCOUNTER> _counter;
	};
}