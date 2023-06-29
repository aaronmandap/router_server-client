'use strict';
'require view';
'require fs';
'require ui';
'require form';
'require rpc';
'require uci';

return view.extend({
    log_types: {
		'router_server': [_('REQUEST'), true],
	},
    logger: null,
	log_map: null,
    load: function () {
		return Promise.all([
			L.resolveDefault(fs.stat('/sbin/logread'), null),
		]);
	},

    render: function(stat) {
        this.logger = stat[0] ? stat[0].path : stat[1] ? stat[1].path : null;
        var m, s, o;
        var this_ = this;

        m = new form.Map('my_server_config', [_('Router Server')], _('This router server is the best server for any router in the world.'));
        s = m.section(form.NamedSection, 'my_server_section');
        o = s.option(form.Flag, 'enable', _('Enable'));
        o.rmempty = false;

		s = m.section(form.TypedSection, '', _(''));
		s.render = L.bind(function () {
			let colE = function (titles) {
				return E('div', {}, titles.map(function (title) {
					return E('div', { 'style': 'display: flex; align-items: center;' }, [
						E('input', {
							'click': ui.createHandlerFn(this_, 'handleClick'),
							'type': 'checkbox',
							'event_type': title.toLowerCase(),
							'checked': 'checked'
						}),
						E('p', {}, _(title))
					]);
				}));
			};
            return E('div', { 'class': 'cbi-section' }, [
                E('h3', { 'class': 'cbi-value-title' }, _('Request Log')),
            ])
		});

        
        this.log_map = new form.Map('', _(''));
		s = this.log_map.section(form.TypedSection, '', _(''));
		s.loglines = [];
        s.load = L.bind(function () {
			let self = this;
			return fs.exec_direct(this_.logger, ['-e', '^']).then(
				function (result) {
					self.loglines = result.trim().split(/\n/);
				},
				function (err) {
					ui.addNotification(null, E('p', {}, _('Unable to load log data: ' + err.message)));
					return '';
				}
			);
		}, s);

		s.render = L.bind(function () {
			let rows_info = [];
			let types = [];
            for (let type in this_.log_types) {
				if (this_.log_types[type][1]) {
					types.push(type);
				}
			}

			const regexTypes = new RegExp(`^.*(${types.join('|')}).*$`);
			const regexNames = /(?<timestamp>(\w+\s+){2}\d+\s+\d+:\d+:\d+\s+\d{4})\s+(?<type>\S+)\.(?<priority>\w+)\s(?<name>[^:]*):\s+(?<data>.+)/mg;
			for (let i = 0; types.length > 0 && i < this.loglines.length; i++) {
				let category = this.loglines[i].match(regexTypes);
				if (category == null) {
					continue;
				}

				for (const match of this.loglines[i].matchAll(regexNames)) {
					let datetime = new Date(Date.parse(match.groups.timestamp));
					let date = datetime.getUTCFullYear() + '-' + (datetime.getUTCMonth() + 1) + '-' + datetime.getUTCDate() + ' ' + datetime.toLocaleDateString('en-en', {'weekday':'short'});
					let options = {
						'hour': '2-digit',
						'minute': '2-digit',
						'second': '2-digit',
                        'hour12': 'false',
					};

					let time = datetime.toLocaleTimeString('en-US', options);

					rows_info.push([
						date,
						time,
						match.groups.data,
					]);
				}
			}

			let table_info = E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr table-titles' }, [
					E('h4', { 'class': 'th' }, _('Date')),
					E('h4', { 'class': 'th' }, _('Time')),
					E('h4', { 'class': 'th', 'style': 'width: 40%' }, _('Request Details')),
				])
			]);
			cbi_update_table(table_info, rows_info);

            let section_view = E('div', { 'style': 'width: 92%; display: block; margin-left: auto; margin-right: auto;' }, [
				table_info
			]);
			return section_view;
        }, s);

		return Promise.all([m.render(), this.log_map.render()]);
    },
});
