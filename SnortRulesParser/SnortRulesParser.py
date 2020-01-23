
from idstools import rule
import json
import re

rules_file = r'C:\Users\user\Downloads\snort3-community-rules\snort3-community-rules\snort3-community.rules'
output_file = r'C:\Users\user\Documents\AV\snort_rules.json'


def parse_content(r):
	try: hasattr(r, 'content')
	except: return []
	m = re.match(r'\"(.*)\"', r.content)
	if not m: return []
	out_content = []
	contents = m.group(1)
	for content in contents.split("|"):
		if len(content) == 0: continue
		m = re.match(r'(\d\d ?)+', content)
		if m: out_content.append({"t" : "bytes", "c": m.group(0)})
		else: 
			m = re.match(r'\d+', content)
			if m: out_content.append({"t" : "decimal", "c": m.group(0)})
			else: out_content.append({"t" : "string", "c": content})
	return out_content

def parse_addresses(r):
	m = re.match(r'\S+ \S+ (\S+) (\S+) (->|<-) (\S+) (\S+)', r.raw)
	if not m: return {}
	else: return {
		"local_network":  m.group(1),
		"local_port": m.group(2),
		"remote_network": m.group(4),
		"remote_port": m.group(5)
		}

rules = []
for r in rule.parse_file(rules_file)[:10]:
	out_rule = {
		"msg": r.msg,
		"action": r.action,
		"content": parse_content(r)
	}
	out_rule.update(parse_addresses(r))
	rules.append(out_rule)

with open(output_file, "w") as f:
	json.dump({"rules": rules}, f)