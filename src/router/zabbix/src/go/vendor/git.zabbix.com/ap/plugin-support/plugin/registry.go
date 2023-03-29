/*
** Zabbix
** Copyright 2001-2022 Zabbix SIA
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions
** of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

package plugin

import (
	"fmt"
	"reflect"
	"regexp"
	"unicode"
)

type Metric struct {
	Plugin      Accessor
	Key         string
	Description string
	UsrPrm      bool
}

var Metrics map[string]*Metric = make(map[string]*Metric)
var Plugins map[string]Accessor = make(map[string]Accessor)

func registerMetric(plugin Accessor, name string, key string, description string) {
	var usrprm bool

	if ok, _ := regexp.MatchString(`^[A-Za-z0-9\._-]+$`, key); !ok {
		panic(fmt.Sprintf(`cannot register metric "%s" having invalid format`, key))
	}

	if 0 == len(description) {
		panic(fmt.Sprintf(`cannot register metric "%s" with empty description`, key))
	}

	if unicode.IsLower([]rune(description)[0]) {
		panic(fmt.Sprintf(`cannot register metric "%s" with description without capital first letter: "%s"`, key, description))
	}

	if description[len(description)-1] != '.' {
		panic(fmt.Sprintf(`cannot register metric "%s" without dot at the end of description: "%s"`, key, description))
	}

	if _, ok := Metrics[key]; ok {
		panic(fmt.Sprintf(`cannot register duplicate metric "%s"`, key))
	}

	t := reflect.TypeOf(plugin)
	for i := 0; i < t.NumMethod(); i++ {
		method := t.Method(i)
		switch method.Name {
		case "Export":
			if _, ok := plugin.(Exporter); !ok {
				panic(fmt.Sprintf(`the "%s" plugin has %s method, but does implement Exporter interface`, name, method.Name))
			}
		case "Collect", "Period":
			if _, ok := plugin.(Collector); !ok {
				panic(fmt.Sprintf(`the "%s" plugin has %s method, but does not implement Collector interface`, name, method.Name))
			}
		case "Watch":
			if _, ok := plugin.(Watcher); !ok {
				panic(fmt.Sprintf(`the "%s" plugin has %s method, but does not implement Watcher interface`, name, method.Name))
			}
		case "Configure", "Validate":
			if _, ok := plugin.(Configurator); !ok {
				panic(fmt.Sprintf(`the "%s" plugin has %s method, but does not implement Configurator interface`, name, method.Name))
			}
		case "Start", "Stop":
			if _, ok := plugin.(Runner); !ok {
				panic(fmt.Sprintf(`the "%s" plugin has %s method, but does not implement Runner interface`, name, method.Name))
			}
		}
	}
	switch plugin.(type) {
	case Exporter, Collector, Runner, Watcher, Configurator:
	default:
		panic(fmt.Sprintf(`plugin "%s" does not implement any plugin interfaces`, name))
	}

	if p, ok := Plugins[name]; ok {
		if p != plugin {
			panic(fmt.Sprintf(`plugin name "%s" has been already registered by other plugin`, name))
		}
	} else {
		Plugins[name] = plugin
		plugin.Init(name)
	}

	if name == "UserParameter" {
		usrprm = true
	} else {
		usrprm = false
	}

	Metrics[key] = &Metric{Plugin: plugin, Key: key, Description: description, UsrPrm: usrprm}
}

func RegisterMetrics(impl Accessor, name string, params ...string) {
	if len(params) < 2 {
		panic("expected at least one metric and its description")
	}
	if len(params)&1 != 0 {
		panic("expected even number of metric and description parameters")
	}
	for i := 0; i < len(params); i += 2 {
		registerMetric(impl, name, params[i], params[i+1])
	}
}

func Get(key string) (acc Accessor, err error) {
	if m, ok := Metrics[key]; ok {
		return m.Plugin, nil
	}
	return nil, UnsupportedMetricError
}

func ClearRegistry() {
	Metrics = make(map[string]*Metric)
	Plugins = make(map[string]Accessor)
}

func GetByName(name string) (acc Accessor, err error) {
	if p, ok := Plugins[name]; ok {
		return p, nil
	}
	return nil, UnsupportedMetricError
}

func ClearUserParamMetrics() (metricsFallback map[string]*Metric) {
	metricsFallback = make(map[string]*Metric)

	for key, metric := range Metrics {
		if metric.UsrPrm {
			metricsFallback[key] = metric
			delete(Metrics, key)
		}
	}

	return
}

func RestoreUserParamMetrics(metrics map[string]*Metric) {
	for key, metric := range metrics {
		Metrics[key] = metric
	}
}
