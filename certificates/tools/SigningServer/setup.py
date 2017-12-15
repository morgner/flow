from setuptools import setup , find_packages

setup(
    name='flaskr',
    #packages=['flaskr'],

    description='Simple microblog example using Flask',
    packages=find_packages(),
    entry_points='''
        [flask.commands]
        initdb=flaskr.flaskr:initdb_command
        ''',

    include_package_data=True,
    install_requires=[
        'flask',
    ],
)

