
# -- Project information -----------------------------------------------------

project = 'micropython-usermod'
copyright = '2019-2020, Zoltán Vörös'
author = 'Zoltán Vörös'

# The full version, including alpha/beta/rc tags
release = '1.622'

# -- General configuration ---------------------------------------------------

extensions = [
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

exclude_patterns = []

html_theme = 'sphinx_rtd_theme'

html_static_path = ['_static']

master_doc = 'index'

author=u'Zoltán Vörös'
copyright=author
language='en'

latex_documents = [
(master_doc, 'usermod.tex', 'Micropython usermod documentation', 
'Zoltán Vörös', 'manual'),
]
