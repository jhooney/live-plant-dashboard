FROM grafana/grafana:11.0.0

RUN echo "Starting first Grafana command"

USER root
# Mostly sourced from volkov labs https://volkovlabs.io/blog/how-to-customize-the-grafana-user-interface-8d70a42dc2b6/

# Update Title
RUN sed -i 's|<title>\[\[.AppTitle\]\]</title>|<title>Plants</title>|g' /usr/share/grafana/public/views/index.html
## Update Title
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|AppTitle="Grafana"|AppTitle="Plant Trending"|g' {} \;

## Update Login Title
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|LoginTitle="Welcome to Grafana"|LoginTitle="Welcome"|g' {} \;

## Remove Documentation, Support, Community in the Footer
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|\[{target:"_blank",id:"documentation".*grafana_footer"}\]|\[\]|g' {} \;

## Remove Edition in the Footer
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|({target:"_blank",id:"license",.*licenseUrl})|()|g' {} \;

## Remove Version in the Footer
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|({target:"_blank",id:"version",.*CHANGELOG.md":void 0})|()|g' {} \;

## Remove News icon
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|..createElement(....,{className:.,onClick:.,iconOnly:!0,icon:"rss","aria-label":"News"})|null|g' {} \;

## Remove Open Source icon
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|.push({target:"_blank",id:"version",text:`${..edition}${.}`,url:..licenseUrl,icon:"external-link-alt"})||g' {} \;

## Remove Cloud and Enterprise categories
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|.id==="enterprise"|.id==="notanenterprise"|g' {} \;
RUN find /usr/share/grafana/public/build/ -name *.js -exec sed -i 's|.id==="cloud"|.id==="notacloud"|g' {} \;

USER grafana

RUN echo "Grafana commands complete"
